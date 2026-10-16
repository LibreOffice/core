using System.Globalization;
using System.Xml.Linq;
using XRay.Office.Core.Geometry;

namespace XRay.Office.OpenDocument;

/// <summary>
/// A <c>draw:transform</c>, read into a matrix.
/// </summary>
/// <remarks>
/// <para>
/// Every ODF application writes a turned, sheared or scaled shape as a transform instead of an
/// <c>svg:x</c>/<c>svg:y</c> pair, so this is not a presentation question: a spreadsheet's
/// watermark and a slide's rotated caption are the same attribute read the same way. It lives
/// here for the reason <c>dotnet/CLAUDE.md</c> gives for <c>XRay.Office.Ooxml/DrawingML</c> — it
/// parses markup and depends on nothing above <c>XRay.Office.Core</c>, so a reader of ODF that
/// serves more than one family belongs one layer above Core rather than inside a family.
/// </para>
/// <para>
/// <strong>ODF's rotation runs the other way from OOXML's.</strong> The angle is in radians and
/// counter-clockwise in a y-up reading, which in the y-down space everything here works in means
/// the matrix is <c>[cos, sin; −sin, cos]</c> — the transpose of what a naive reading gives.
/// Measured on <c>shape-geometry.odp</c>, which LibreOffice wrote by converting a deck whose
/// rectangle is rotated 30° clockwise: it comes out as
/// <c>rotate (-0.523598775598299) translate (3.515cm 10.33cm)</c>, and only this reading puts the
/// shape's centre back at the 5.0795 cm, 12.6995 cm the OOXML original states.
/// </para>
/// <para>
/// The operations compose right to left, as they do in SVG: the last one written is applied last.
/// Only <c>rotate</c>, <c>translate</c>, <c>scale</c> and <c>skewX</c> are read; <c>matrix</c> is
/// rare enough in real files that leaving it out is honest about coverage rather than a gap worth
/// filling blind.
/// </para>
/// </remarks>
public static class OdfTransform
{
    /// <summary>The element's <c>draw:transform</c> as a matrix, or null when it states none.</summary>
    /// <param name="element">The shape element.</param>
    public static AffineTransform? Read(XElement element)
    {
        ArgumentNullException.ThrowIfNull(element);

        return Parse(element.Attribute(XName.Get("transform", OdfNamespaces.Draw))?.Value);
    }

    /// <summary>The transform text as a matrix, or null when there is nothing in it.</summary>
    /// <param name="text">The attribute's value.</param>
    public static AffineTransform? Parse(string? text)
    {
        if (string.IsNullOrWhiteSpace(text)) return null;

        AffineTransform result = AffineTransform.Identity;
        bool any = false;

        foreach ((string name, string[] arguments) in Operations(text))
        {
            AffineTransform step = name switch
            {
                "translate" when arguments.Length >= 1 => AffineTransform.Translation(
                    Emu(arguments[0]), arguments.Length > 1 ? Emu(arguments[1]) : 0),
                "rotate" when arguments.Length >= 1 => Rotation(Number(arguments[0])),
                "scale" when arguments.Length >= 1 => AffineTransform.Scale(
                    Number(arguments[0]),
                    arguments.Length > 1 ? Number(arguments[1]) : Number(arguments[0])),
                "skewX" when arguments.Length >= 1 => new AffineTransform(
                    1, 0, Math.Tan(Number(arguments[0])), 1, 0, 0),
                _ => AffineTransform.Identity,
            };

            result = AffineTransform.Concat(result, step);
            any = true;
        }

        return any ? result : null;
    }

    /// <summary>
    /// A transform argument that is a length, in EMUs.
    /// </summary>
    /// <remarks>
    /// A bare number is a hundredth of a millimetre, which is ODF's unitless default and what
    /// <see cref="OdfValue.ParseLength"/> already assumes.
    /// </remarks>
    private static double Emu(string token)
        => OdfValue.ParseLength(token) is { } length ? length.Emu : 0;

    /// <summary>
    /// A transform argument that is a plain number: an angle in radians, or a scale factor.
    /// </summary>
    /// <remarks>
    /// Read as a number and never as a length, which is the whole reason the two are separate.
    /// <see cref="OdfValue.ParseLength"/> takes a unitless value for hundredths of a millimetre,
    /// so putting <c>rotate (-0.5236)</c> through it rounds the angle to −1 and then treats it as
    /// −360 radians — a rotation of about −106 degrees once wrapped, which lands the shape in a
    /// plausible-looking wrong place rather than an obviously wrong one.
    /// </remarks>
    private static double Number(string token)
        => double.TryParse(token, NumberStyles.Float, CultureInfo.InvariantCulture, out double value)
            ? value
            : 0;

    /// <summary>
    /// ODF's rotation, expressed in a y-down space.
    /// </summary>
    /// <remarks>
    /// Public because a shape's <c>draw:text-rotate-angle</c> is stated in the same sense and is
    /// composed against the shape's own placement rather than parsed out of a transform.
    /// </remarks>
    /// <param name="radians">The angle, in ODF's own sense.</param>
    public static AffineTransform Rotation(double radians)
    {
        double cos = Math.Cos(radians);
        double sin = Math.Sin(radians);
        return new AffineTransform(cos, -sin, sin, cos, 0, 0);
    }

    /// <summary>
    /// The operations in a <c>draw:transform</c>, with their arguments still as written.
    /// </summary>
    /// <remarks>
    /// Unparsed, because what an argument means depends on the operation: <c>translate</c> takes
    /// lengths and <c>rotate</c> takes a bare number of radians, and the two readings are not
    /// interchangeable.
    /// </remarks>
    private static IEnumerable<(string Name, string[] Arguments)> Operations(string text)
    {
        int at = 0;
        while (at < text.Length)
        {
            while (at < text.Length && (char.IsWhiteSpace(text[at]) || text[at] == ',')) at++;

            int nameStart = at;
            while (at < text.Length && char.IsLetter(text[at])) at++;
            if (at == nameStart) yield break;

            string name = text[nameStart..at];

            while (at < text.Length && text[at] != '(') at++;
            int open = at + 1;
            int close = text.IndexOf(')', open);
            if (close < 0) yield break;

            string[] arguments = text[open..close].Split(
                [' ', ',', '\t', '\n', '\r'], StringSplitOptions.RemoveEmptyEntries);

            at = close + 1;
            yield return (name, arguments);
        }
    }
}
