using System.Text;

namespace Paperless.OpenDocument;

/// <summary>Encoding rules for the names ODF uses in attributes.</summary>
public static class OdfNames
{
    /// <summary>
    /// Decodes a style name back to the name a user sees: <c>Text_20_body</c> becomes
    /// <c>Text body</c>.
    /// </summary>
    /// <remarks>
    /// <para>
    /// Style names are referenced from attributes that must be valid XML names, so ODF
    /// escapes any character that is not — a space, a bracket, an ampersand — as an
    /// underscore, the character's hex code point, and another underscore. LibreOffice
    /// reverses this on import, and so does Paperless: the escaping is an artefact of the
    /// serialisation, not part of the style's identity, and reporting
    /// <c>Heading_20_1</c> to a caller looking for "Heading 1" would be needlessly hostile.
    /// </para>
    /// <para>
    /// A name containing an underscore that is not part of an escape — <c>My_Style</c> — is
    /// returned unchanged, because the escape pattern requires hex digits between the two
    /// underscores.
    /// </para>
    /// </remarks>
    public static string Decode(string name)
    {
        ArgumentNullException.ThrowIfNull(name);
        int first = name.IndexOf('_', StringComparison.Ordinal);
        if (first < 0) return name;

        StringBuilder result = new(name.Length);
        int position = 0;
        while (position < name.Length)
        {
            int underscore = name.IndexOf('_', position);
            if (underscore < 0 || underscore == name.Length - 1)
            {
                result.Append(name, position, name.Length - position);
                break;
            }

            int closing = name.IndexOf('_', underscore + 1);
            // ODF writes between two and six hex digits, matching the widest code point.
            int digits = closing - underscore - 1;
            if (closing > 0 && digits is >= 2 and <= 6
                && uint.TryParse(name.AsSpan(underscore + 1, digits),
                                 System.Globalization.NumberStyles.HexNumber,
                                 System.Globalization.CultureInfo.InvariantCulture,
                                 out uint codePoint)
                && codePoint is > 0 and <= 0x10FFFF
                && !(codePoint is >= 0xD800 and <= 0xDFFF))
            {
                result.Append(name, position, underscore - position);
                result.Append(char.ConvertFromUtf32((int)codePoint));
                position = closing + 1;
            }
            else
            {
                result.Append(name, position, underscore - position + 1);
                position = underscore + 1;
            }
        }
        return result.ToString();
    }
}
