namespace Paperless.WordProcessing.Rtf;

/// <summary>What an <see cref="RtfToken"/> is.</summary>
public enum RtfTokenKind
{
    /// <summary>End of input.</summary>
    End = 0,

    /// <summary>A <c>{</c>: the start of a group.</summary>
    GroupStart,

    /// <summary>A <c>}</c>: the end of a group.</summary>
    GroupEnd,

    /// <summary>
    /// A control word such as <c>\par</c> or <c>\fs24</c>, with its optional numeric parameter.
    /// </summary>
    ControlWord,

    /// <summary>
    /// A control symbol such as <c>\*</c>, <c>\~</c> or an escaped <c>\{</c>: a backslash
    /// followed by a single non-alphabetic character.
    /// </summary>
    ControlSymbol,

    /// <summary>
    /// Literal text. Held as bytes rather than characters because the encoding is decided by
    /// control words that may appear anywhere, so decoding cannot happen in the tokeniser.
    /// </summary>
    Text,

    /// <summary>
    /// A single byte from a <c>\'hh</c> escape, to be decoded in whatever code page is in force.
    /// </summary>
    EscapedByte,

    /// <summary>
    /// A Unicode code point from a <c>\uN</c> control word, already decoded and needing no code
    /// page.
    /// </summary>
    UnicodeCharacter,
}

/// <summary>One token from an RTF stream.</summary>
/// <param name="Kind">What kind of token this is.</param>
/// <param name="Name">The control word's name, without its backslash. Empty for other kinds.</param>
/// <param name="Parameter">
/// The control word's numeric parameter, or null when it has none. The distinction matters:
/// <c>\b</c> means bold on and <c>\b0</c> means bold off, so an absent parameter is not zero.
/// </param>
/// <param name="Symbol">The control symbol's character.</param>
/// <param name="Bytes">The literal bytes, for <see cref="RtfTokenKind.Text"/>.</param>
/// <param name="CodePoint">The code point, for <see cref="RtfTokenKind.UnicodeCharacter"/>.</param>
public readonly record struct RtfToken(
    RtfTokenKind Kind,
    string Name = "",
    int? Parameter = null,
    char Symbol = '\0',
    ReadOnlyMemory<byte> Bytes = default,
    int CodePoint = 0);

/// <summary>
/// Splits an RTF stream into groups, control words and text.
/// </summary>
/// <remarks>
/// <para>
/// RTF is a byte stream with no declared encoding: an <c>\ansicpg</c> control word somewhere near
/// the start says how to interpret <c>\'hh</c> escapes and literal bytes, and a
/// <c>\fcharset</c> on a font can override it mid-document. So the tokeniser hands text back as
/// <em>bytes</em> and leaves decoding to the reader, which knows what is in force where.
/// Decoding here would have to guess, and guessing is how a document comes out mojibake.
/// </para>
/// <para>
/// Three details in the grammar are easy to get wrong and each is handled explicitly below: a
/// single space after a control word is its delimiter and not text; a bare carriage return or
/// line feed is <em>ignored</em> rather than being a line break; and a backslash immediately
/// followed by a newline means <c>\par</c>.
/// </para>
/// </remarks>
public sealed class RtfTokeniser
{
    /// <summary>
    /// The longest control word RTF permits, per the specification. A longer run of letters is
    /// malformed, and stopping at the limit keeps a corrupt file from consuming the document as
    /// one enormous name.
    /// </summary>
    public const int MaxControlWordLength = 32;

    /// <summary>
    /// The most digits a parameter may have. Ten covers <see cref="int"/>'s range; more means the
    /// file is malformed.
    /// </summary>
    public const int MaxParameterDigits = 10;

    private readonly byte[] _data;
    private int _position;

    /// <summary>Creates a tokeniser over a whole RTF document.</summary>
    public RtfTokeniser(byte[] data)
    {
        ArgumentNullException.ThrowIfNull(data);
        _data = data;
    }

    /// <summary>How far through the input the tokeniser is, for diagnostics.</summary>
    public int Position => _position;

    /// <summary>Reads the next token, or <see cref="RtfTokenKind.End"/> at the end of input.</summary>
    public RtfToken Read()
    {
        while (_position < _data.Length)
        {
            byte current = _data[_position];

            switch (current)
            {
                case (byte)'{':
                    _position++;
                    return new RtfToken(RtfTokenKind.GroupStart);

                case (byte)'}':
                    _position++;
                    return new RtfToken(RtfTokenKind.GroupEnd);

                case (byte)'\\':
                    return ReadControl();

                // A bare newline is not content. RTF uses it purely to keep lines short, and
                // treating it as text inserts a space or a break into every long paragraph.
                case (byte)'\r' or (byte)'\n':
                    _position++;
                    continue;

                // A literal NUL appears in damaged files; it is not text either.
                case 0:
                    _position++;
                    continue;

                default:
                    return ReadText();
            }
        }

        return new RtfToken(RtfTokenKind.End);
    }

    private RtfToken ReadControl()
    {
        // Past the backslash.
        _position++;
        if (_position >= _data.Length) return new RtfToken(RtfTokenKind.End);

        byte first = _data[_position];

        // A backslash followed by a newline is a paragraph break, not an escape.
        if (first is (byte)'\r' or (byte)'\n')
        {
            _position++;
            return new RtfToken(RtfTokenKind.ControlWord, "par");
        }

        if (!IsAsciiLetter(first))
        {
            _position++;

            // \'hh is the one control symbol with an argument: a byte in the current code page.
            if (first == (byte)'\'')
            {
                int value = ReadHexByte();
                return value < 0
                    ? new RtfToken(RtfTokenKind.ControlSymbol, Symbol: '\'')
                    : new RtfToken(RtfTokenKind.EscapedByte, CodePoint: value);
            }

            return new RtfToken(RtfTokenKind.ControlSymbol, Symbol: (char)first);
        }

        int nameStart = _position;
        while (_position < _data.Length
               && IsAsciiLetter(_data[_position])
               && _position - nameStart < MaxControlWordLength)
        {
            _position++;
        }

        string name = System.Text.Encoding.ASCII.GetString(_data, nameStart, _position - nameStart);

        int? parameter = null;
        bool negative = false;
        if (_position < _data.Length && _data[_position] == (byte)'-')
        {
            negative = true;
            _position++;
        }

        if (_position < _data.Length && IsAsciiDigit(_data[_position]))
        {
            long value = 0;
            int digits = 0;
            while (_position < _data.Length && IsAsciiDigit(_data[_position]) && digits < MaxParameterDigits)
            {
                value = (value * 10) + (_data[_position] - (byte)'0');
                _position++;
                digits++;
            }
            parameter = (int)Math.Clamp(negative ? -value : value, int.MinValue, int.MaxValue);
        }
        else if (negative)
        {
            // A stray minus with no digits. Treat the word as parameterless rather than
            // swallowing the character.
            _position--;
        }

        // Exactly one space after a control word is the delimiter and is not text. A second
        // space is text, which is why this consumes one and not a run.
        if (_position < _data.Length && _data[_position] == (byte)' ') _position++;

        // \uN is a code point, and it is worth resolving here so the reader does not have to
        // treat it differently from any other character. RTF writes it signed, so a code point
        // above 32767 arrives negative.
        if (name == "u" && parameter is { } codePoint)
        {
            if (codePoint < 0) codePoint += 65536;
            if (codePoint is > 0 and <= 0x10FFFF && codePoint is not (>= 0xD800 and <= 0xDFFF))
                return new RtfToken(RtfTokenKind.UnicodeCharacter, name, parameter, CodePoint: codePoint);
        }

        return new RtfToken(RtfTokenKind.ControlWord, name, parameter);
    }

    private RtfToken ReadText()
    {
        int start = _position;
        while (_position < _data.Length)
        {
            byte current = _data[_position];
            if (current is (byte)'{' or (byte)'}' or (byte)'\\' or (byte)'\r' or (byte)'\n' or 0) break;
            _position++;
        }
        return new RtfToken(RtfTokenKind.Text, Bytes: _data.AsMemory(start, _position - start));
    }

    private int ReadHexByte()
    {
        if (_position + 1 >= _data.Length) return -1;

        int high = HexDigit(_data[_position]);
        int low = HexDigit(_data[_position + 1]);
        if (high < 0 || low < 0) return -1;

        _position += 2;
        return (high << 4) | low;
    }

    private static int HexDigit(byte value) => value switch
    {
        >= (byte)'0' and <= (byte)'9' => value - (byte)'0',
        >= (byte)'a' and <= (byte)'f' => value - (byte)'a' + 10,
        >= (byte)'A' and <= (byte)'F' => value - (byte)'A' + 10,
        _ => -1,
    };

    private static bool IsAsciiLetter(byte value)
        => value is >= (byte)'a' and <= (byte)'z' or >= (byte)'A' and <= (byte)'Z';

    private static bool IsAsciiDigit(byte value) => value is >= (byte)'0' and <= (byte)'9';
}
