using System.Globalization;
using System.Text;
using Paperless.Core.Geometry;
using Paperless.Core.Graphics;
using Paperless.Rendering.Fills;

namespace Paperless.Rendering.Pdf;

/// <summary>
/// Writes a gradient as a PDF shading dictionary and the function behind it.
/// </summary>
/// <remarks>
/// <para>
/// The shape follows <c>PDFWriterImpl::writeGradientFunction</c>
/// (<c>vcl/source/pdf/pdfwriter_impl.cxx:7965</c>), which is the closest thing to a
/// specification for what a correct shading here looks like: a linear gradient is
/// <c>/ShadingType 2</c> with <c>/Coords</c> naming the two ends of the axis and
/// <c>/Extend [true true]</c> so the colour continues past them.
/// </para>
/// <para>
/// Two deliberate departures from it, both recorded in this library's <c>TODO.md</c>:
/// </para>
/// <list type="bullet">
///   <item>
///     <b>A radial gradient is <c>/ShadingType 3</c>.</b> LibreOffice has no radial shading
///     at all — <c>lcl_canUsePDFAxialShading</c> (<c>pdfwriter_impl2.cxx:1061</c>) accepts
///     only <c>LINEAR</c> and <c>AXIAL</c>, and everything else falls to a sampled
///     <c>/ShadingType 1</c> whose function stream is one RGB triple per point of the
///     shape. Type 3 states the same picture as two circles and a ramp.
///   </item>
///   <item>
///     <b>The function is exponential (<c>/FunctionType 2</c>), stitched
///     (<c>/FunctionType 3</c>) when there are more than two stops</b>, where LibreOffice
///     writes a sampled <c>/FunctionType 0</c> with two or three samples. Both are exact for
///     two stops; only the stitched form can state a multi-stop ramp without sampling it,
///     which is what DrawingML's <c>a:gsLst</c> and ODF 1.3's <c>loext:gradient-stop</c>
///     both express and neither of the two-colour attributes can.
///   </item>
/// </list>
/// </remarks>
internal static class PdfShadings
{
    /// <summary>
    /// Writes the shading for a gradient and returns its object number.
    /// </summary>
    /// <param name="writer">The document being built.</param>
    /// <param name="gradient">The gradient. Its kind must have a native form.</param>
    /// <param name="pageHeight">
    /// The page height in points, because a shading's coordinates are stated in the user
    /// space the <c>sh</c> operator paints in and every other coordinate this backend writes
    /// is converted the same way.
    /// </param>
    /// <param name="alphaOnly">
    /// When true the shading is written in <c>DeviceGray</c> with each stop's <em>alpha</em> as
    /// its grey level, which is the luminosity a soft mask reads. A PDF shading has no alpha
    /// channel of its own, so a gradient that fades is two shadings: this one masking the other.
    /// </param>
    public static int Write(
        PdfDocumentWriter writer, GradientPaint gradient, double pageHeight, bool alphaOnly = false)
    {
        ArgumentNullException.ThrowIfNull(writer);
        ArgumentNullException.ThrowIfNull(gradient);

        IReadOnlyList<GradientStop> stops = Gradients.Normalise(gradient.Stops);
        int function = Function(writer, stops, alphaOnly);
        string space = alphaOnly ? "/DeviceGray" : "/DeviceRGB";

        double x0 = gradient.Start.X.Points;
        double y0 = pageHeight - gradient.Start.Y.Points;
        double x1 = gradient.End.X.Points;
        double y1 = pageHeight - gradient.End.Y.Points;

        if (gradient.Kind == GradientKind.Linear)
        {
            return writer.Add(
                $"<</ShadingType 2/ColorSpace{space}"
                + $"/Coords[{N(x0)} {N(y0)} {N(x1)} {N(y1)}]"
                + $"/Function {function} 0 R/Extend[true true]>>");
        }

        // Start is the centre and End a point on the outer edge, so the radius is the distance
        // between them. A zero radius would make the whole shading degenerate, and a shape given
        // a gradient with no extent is better drawn as its outer colour than not at all.
        double radius = Math.Sqrt(((x1 - x0) * (x1 - x0)) + ((y1 - y0) * (y1 - y0)));
        if (radius <= 0) radius = 1;

        return writer.Add(
            $"<</ShadingType 3/ColorSpace{space}"
            + $"/Coords[{N(x0)} {N(y0)} 0 {N(x0)} {N(y0)} {N(radius)}]"
            + $"/Function {function} 0 R/Extend[true true]>>");
    }

    /// <summary>
    /// The function taking a shading's parameter to a colour.
    /// </summary>
    /// <remarks>
    /// One exponential piece per pair of adjacent stops, with <c>/N 1</c> — a linear ramp,
    /// which is what both formats mean by a stop list. Several pieces are stitched with a
    /// type 3, whose <c>/Bounds</c> are the interior stop offsets and whose <c>/Encode</c>
    /// re-maps each piece's own slice of the domain back onto 0 to 1.
    /// </remarks>
    private static int Function(PdfDocumentWriter writer, IReadOnlyList<GradientStop> stops, bool alphaOnly)
    {
        if (stops.Count == 2)
        {
            return writer.Add(
                $"<</FunctionType 2/Domain[0 1]/C0[{Components(stops[0].Colour, alphaOnly)}]"
                + $"/C1[{Components(stops[1].Colour, alphaOnly)}]/N 1>>");
        }

        StringBuilder functions = new();
        StringBuilder bounds = new();
        StringBuilder encode = new();

        for (int i = 1; i < stops.Count; i++)
        {
            int piece = writer.Add(
                $"<</FunctionType 2/Domain[0 1]/C0[{Components(stops[i - 1].Colour, alphaOnly)}]"
                + $"/C1[{Components(stops[i].Colour, alphaOnly)}]/N 1>>");

            if (i > 1)
            {
                functions.Append(' ');
                bounds.Append(' ');
                encode.Append(' ');
            }

            functions.Append(CultureInfo.InvariantCulture, $"{piece} 0 R");
            encode.Append("0 1");

            if (i < stops.Count - 1) bounds.Append(N(stops[i].Offset));
        }

        return writer.Add(
            $"<</FunctionType 3/Domain[0 1]/Functions[{functions}]"
            + $"/Bounds[{bounds.ToString().Trim()}]/Encode[{encode}]>>");
    }

    private static string Components(Colour colour, bool alphaOnly)
        => alphaOnly
            ? PdfSyntax.Component(colour.A)
            : $"{PdfSyntax.Component(colour.R)} {PdfSyntax.Component(colour.G)} {PdfSyntax.Component(colour.B)}";

    private static string N(double value) => PdfSyntax.Number(value);
}
