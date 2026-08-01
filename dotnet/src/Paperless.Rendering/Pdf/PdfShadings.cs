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
    /// <param name="extent">
    /// What the shading has to cover, in gradient space, or null when nothing needs it. Only a
    /// repeating <see cref="SpreadMethod"/> reads it — a padded gradient covers the plane by
    /// <c>/Extend</c> alone, whereas a repeat has to be spelled as an axis lengthened over as
    /// many periods as the shape spans.
    /// </param>
    public static int Write(
        PdfDocumentWriter writer,
        GradientPaint gradient,
        double pageHeight,
        bool alphaOnly = false,
        DocRect? extent = null)
    {
        ArgumentNullException.ThrowIfNull(writer);
        ArgumentNullException.ThrowIfNull(gradient);

        IReadOnlyList<GradientStop> stops = Gradients.Normalise(gradient.Stops);
        string space = alphaOnly ? "/DeviceGray" : "/DeviceRGB";

        (int before, int after) = extent is { } box && gradient.Spread != SpreadMethod.Pad
            ? Gradients.Periods(gradient, box)
            : (0, 0);

        int function = before == 0 && after == 0
            ? Function(writer, stops, alphaOnly)
            : Repeating(writer, stops, alphaOnly, gradient.Spread, before, after);

        string domain = before == 0 && after == 0
            ? "[0 1]"
            : $"[{N(-before)} {N(1 + after)}]";

        double x0 = gradient.Start.X.Points;
        double y0 = pageHeight - gradient.Start.Y.Points;
        double x1 = gradient.End.X.Points;
        double y1 = pageHeight - gradient.End.Y.Points;

        if (gradient.Kind == GradientKind.Linear)
        {
            // Lengthening the axis by a whole period each way and widening the domain to match
            // is what makes a repeat expressible at all — see Gradients.Periods for why /Extend
            // cannot do it.
            double ax = x0 - (before * (x1 - x0));
            double ay = y0 - (before * (y1 - y0));
            double bx = x1 + (after * (x1 - x0));
            double by = y1 + (after * (y1 - y0));

            return writer.Add(
                $"<</ShadingType 2/ColorSpace{space}"
                + $"/Coords[{N(ax)} {N(ay)} {N(bx)} {N(by)}]/Domain{domain}"
                + $"/Function {function} 0 R/Extend[true true]>>");
        }

        // Start is the centre and End a point on the outer edge, so the radius is the distance
        // between them. A zero radius would make the whole shading degenerate, and a shape given
        // a gradient with no extent is better drawn as its outer colour than not at all.
        double radius = Math.Sqrt(((x1 - x0) * (x1 - x0)) + ((y1 - y0) * (y1 - y0)));
        if (radius <= 0) radius = 1;

        // A focal radial is the two-circle form type 3 already takes: the inner circle collapsed
        // to a point at the focus rather than at the centre. Nothing else changes — the same
        // function, the same extension — which is why a focus costs a coordinate in the IR and
        // no second code path here.
        double fx = gradient.Focus is { } focus ? focus.X.Points : x0;
        double fy = gradient.Focus is { } f ? pageHeight - f.Y.Points : y0;

        return writer.Add(
            $"<</ShadingType 3/ColorSpace{space}"
            + $"/Coords[{N(fx)} {N(fy)} 0 {N(x0)} {N(y0)} {N(radius * (1 + after))}]/Domain{domain}"
            + $"/Function {function} 0 R/Extend[true true]>>");
    }

    /// <summary>
    /// A stitching function repeating a ramp over a domain of several whole periods.
    /// </summary>
    /// <remarks>
    /// One sub-function per period, each the whole ramp, with <c>/Encode</c> saying which way
    /// round it runs: <c>[0 1]</c> for every period of a <see cref="SpreadMethod.Repeat"/>, and
    /// alternating <c>[0 1]</c> and <c>[1 0]</c> for a <see cref="SpreadMethod.Reflect"/> — a
    /// reversed encode is how PDF spells a mirrored copy without a second function object. The
    /// period holding the ramp itself is always the forward one, so period <c>-1</c> and period
    /// <c>1</c> are both reversed and the reflection is symmetric about both ends.
    /// </remarks>
    private static int Repeating(
        PdfDocumentWriter writer,
        IReadOnlyList<GradientStop> stops,
        bool alphaOnly,
        SpreadMethod spread,
        int before,
        int after)
    {
        int ramp = Function(writer, stops, alphaOnly);

        StringBuilder functions = new();
        StringBuilder bounds = new();
        StringBuilder encode = new();

        for (int period = -before; period <= after; period++)
        {
            if (period > -before)
            {
                functions.Append(' ');
                bounds.Append(' ');
                encode.Append(' ');
            }

            functions.Append(CultureInfo.InvariantCulture, $"{ramp} 0 R");

            bool reversed = spread == SpreadMethod.Reflect && (period & 1) != 0;
            encode.Append(reversed ? "1 0" : "0 1");

            if (period < after) bounds.Append(N(period + 1));
        }

        return writer.Add(
            $"<</FunctionType 3/Domain[{N(-before)} {N(1 + after)}]/Functions[{functions}]"
            + $"/Bounds[{bounds.ToString().Trim()}]/Encode[{encode}]>>");
    }

    /// <summary>
    /// Writes a <see cref="MeshPaint"/> as a free-form Gouraud triangle mesh, and returns its
    /// object number.
    /// </summary>
    /// <remarks>
    /// <para>
    /// <c>/ShadingType 4</c> ([PDF 32000-1] 8.7.4.5.5): a stream of vertices, each an edge flag,
    /// a packed coordinate pair and a colour, with flag 0 starting a fresh triangle. Every
    /// triangle is written as three flag-0 vertices rather than as a strip or a fan, which
    /// triples nothing that matters — a fan built from a path's own boundary shares no edge with
    /// the next fan anyway — and removes the one thing a reader can get wrong about the format.
    /// </para>
    /// <para>
    /// <b>Every field is a whole number of bytes.</b> <c>/BitsPerFlag 8</c>,
    /// <c>/BitsPerCoordinate 32</c> and <c>/BitsPerComponent 8</c> make a vertex 1 + 4 + 4 + 3
    /// bytes, so no vertex straddles a byte and the "each vertex begins on a byte boundary" rule
    /// is satisfied by construction rather than by padding. The alternative — 16-bit
    /// coordinates — quantises a page-wide mesh to a fifteenth of a millimetre, which is visible
    /// as a ragged boundary on the very shape this paint exists to draw.
    /// </para>
    /// <para>
    /// <b>Coordinates are unsigned integers decoded through <c>/Decode</c>.</b> The range is the
    /// mesh's own bounding box, so the whole 32-bit resolution is spent on the shape rather than
    /// on the page, and a mesh smaller than a point is still exact.
    /// </para>
    /// </remarks>
    /// <param name="writer">The document being built.</param>
    /// <param name="mesh">The mesh.</param>
    /// <param name="pageHeight">The page height in points; a shading paints in user space.</param>
    /// <param name="alphaOnly">
    /// When true the mesh is written in <c>DeviceGray</c> with each vertex's <em>alpha</em> as
    /// its grey level, which is what a luminosity soft mask reads. A shading has no alpha
    /// channel, so a mesh with a translucent vertex is two meshes.
    /// </param>
    /// <returns>The shading's object number, or zero when the mesh draws nothing.</returns>
    public static int WriteMesh(
        PdfDocumentWriter writer, MeshPaint mesh, double pageHeight, bool alphaOnly = false)
    {
        ArgumentNullException.ThrowIfNull(writer);
        ArgumentNullException.ThrowIfNull(mesh);

        if (Meshes.Bounds(mesh) is not { } bounds) return 0;

        double left = bounds.Left.Points;
        double right = bounds.Right.Points;
        double top = pageHeight - bounds.Bottom.Points;
        double bottom = pageHeight - bounds.Top.Points;

        // A mesh with no extent on an axis would divide by zero below; widening it by a point
        // leaves every vertex on that axis decoding to the same place, which is what it is.
        if (right - left < 1e-9) right = left + 1;
        if (bottom - top < 1e-9) bottom = top + 1;

        List<byte> stream = [];
        int components = alphaOnly ? 1 : 3;

        foreach (MeshTriangle triangle in Meshes.Valid(mesh))
        {
            foreach (int index in (ReadOnlySpan<int>)[triangle.A, triangle.B, triangle.C])
            {
                MeshVertex vertex = mesh.Vertices[index];

                stream.Add(0);
                Coordinate(stream, (vertex.Position.X.Points - left) / (right - left));
                Coordinate(stream, (pageHeight - vertex.Position.Y.Points - top) / (bottom - top));

                if (alphaOnly)
                {
                    stream.Add(vertex.Colour.A);
                    continue;
                }

                stream.Add(vertex.Colour.R);
                stream.Add(vertex.Colour.G);
                stream.Add(vertex.Colour.B);
            }
        }

        if (stream.Count == 0) return 0;

        string decode = $"[{N(left)} {N(right)} {N(top)} {N(bottom)}"
            + string.Concat(Enumerable.Repeat(" 0 1", components))
            + "]";

        int shading = writer.Reserve();
        writer.SetStream(
            shading,
            $"/ShadingType 4/ColorSpace{(alphaOnly ? "/DeviceGray" : "/DeviceRGB")}"
            + $"/BitsPerCoordinate 32/BitsPerComponent 8/BitsPerFlag 8/Decode{decode}",
            [.. stream],
            compress: true);

        return shading;
    }

    /// <summary>Packs a 0-to-1 position as the big-endian 32-bit integer <c>/Decode</c> expands.</summary>
    private static void Coordinate(List<byte> stream, double fraction)
    {
        uint value = (uint)Math.Clamp(Math.Round(Math.Clamp(fraction, 0, 1) * uint.MaxValue), 0, uint.MaxValue);

        stream.Add((byte)(value >> 24));
        stream.Add((byte)(value >> 16));
        stream.Add((byte)(value >> 8));
        stream.Add((byte)value);
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
