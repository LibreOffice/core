using System.Buffers.Binary;
using System.Globalization;
using System.Text;
using Paperless.Containers;
using Paperless.Core.Documents;
using Paperless.Text.Encodings;

namespace Paperless.MsBinary.PropertySets;

/// <summary>
/// Reads the OLE property sets that carry document metadata in legacy binary files:
/// the summary information and document summary information streams.
/// </summary>
/// <remarks>
/// <para>
/// Both stream names begin with the character U+0005, which is part of the name rather
/// than an escape sequence — hence the explicit construction in
/// <see cref="SummaryInformationStreamName"/>. Property identifiers are numeric, and
/// their meaning depends on which of the two property sets they appear in, so the same
/// identifier means different things in each. See
/// <c>dotnet/research/05-infrastructure.md</c> section G.
/// </para>
/// <para>
/// One reader serves DOC, XLS and PPT: the property sets are an OLE facility rather than anything
/// Word, Excel or PowerPoint defined, so all three store their metadata identically and only the
/// accompanying content streams differ.
/// </para>
/// </remarks>
public static class OlePropertySetReader
{
    /// <summary>The name of the summary information stream, including its leading U+0005.</summary>
    public const string SummaryInformationStreamName = "\u0005SummaryInformation";

    /// <summary>The name of the document summary information stream, including its leading U+0005.</summary>
    public const string DocumentSummaryInformationStreamName = "\u0005DocumentSummaryInformation";

    /// <summary>
    /// The largest property-set stream Paperless will read.
    /// </summary>
    /// <remarks>
    /// Metadata is small by nature, and every property in a set is addressed by an offset the file
    /// states rather than reached by reading forwards — so without a cap a hostile stream claiming to
    /// be enormous would be buffered whole before any of it could be validated.
    /// </remarks>
    public const int MaxStreamBytes = 16 * 1024 * 1024;

    /// <summary>How many properties are read from one section before the rest are ignored.</summary>
    public const int MaxPropertiesPerSection = 4096;

    /// <summary>The byte-order mark a property-set stream begins with.</summary>
    private const ushort ByteOrderMarker = 0xFFFE;

    /// <summary>
    /// The pseudo code page meaning "this section's strings are UTF-16 rather than bytes".
    /// </summary>
    private const int Utf16CodePage = 1200;

    /// <summary>
    /// Reads both property-set streams from a compound file, if present, and maps them
    /// onto the normalised metadata model.
    /// </summary>
    /// <remarks>
    /// A missing or malformed stream yields empty metadata rather than an error: a document whose
    /// text reads perfectly well should not fail over its properties.
    /// </remarks>
    public static DocumentMetadata Read(IPackage package)
    {
        ArgumentNullException.ThrowIfNull(package);

        OlePropertySet summary = ReadSet(package, SummaryInformationStreamName);
        OlePropertySet document = ReadSet(package, DocumentSummaryInformationStreamName);

        Dictionary<string, object?> custom = [];
        foreach ((string name, object? value) in document.NamedProperties) custom[name] = value;

        return new DocumentMetadata
        {
            Title = summary.Text(SummaryId.Title),
            Subject = summary.Text(SummaryId.Subject),
            Author = summary.Text(SummaryId.Author),
            LastModifiedBy = summary.Text(SummaryId.LastAuthor),
            Description = summary.Text(SummaryId.Comments),
            Keywords = SplitKeywords(summary.Text(SummaryId.Keywords)),
            Category = document.Text(DocumentSummaryId.Category),
            Created = summary.Timestamp(SummaryId.Created),
            Modified = summary.Timestamp(SummaryId.LastSaved),
            Printed = summary.Timestamp(SummaryId.LastPrinted),
            GeneratorApplication = summary.Text(SummaryId.ApplicationName),
            RevisionNumber = RevisionOf(summary.Text(SummaryId.RevisionNumber)),
            TotalEditingTime = summary.Duration(SummaryId.EditingTime),
            Statistics = StatisticsOf(summary, document),
            CustomProperties = custom,
        };
    }

    /// <summary>
    /// The counts the two property sets record, or null when neither records any.
    /// </summary>
    /// <remarks>
    /// The counts are split across the sets — pages, words and characters in the summary, paragraphs
    /// in the document summary — because the summary set predates the second and could not be
    /// extended. Returning null rather than an all-null record keeps "the document said nothing"
    /// distinguishable from "the document said nothing useful".
    /// </remarks>
    private static DocumentStatistics? StatisticsOf(OlePropertySet summary, OlePropertySet document)
    {
        int? pages = summary.Integer(SummaryId.PageCount);
        int? words = summary.Integer(SummaryId.WordCount);
        int? characters = summary.Integer(SummaryId.CharacterCount);
        int? paragraphs = document.Integer(DocumentSummaryId.ParagraphCount);

        if (pages is null && words is null && characters is null && paragraphs is null) return null;

        return new DocumentStatistics
        {
            PageCount = pages,
            WordCount = words,
            CharacterCount = characters,
            ParagraphCount = paragraphs,
        };
    }

    /// <summary>
    /// Splits the keywords property, which is one string however many keywords it holds.
    /// </summary>
    /// <remarks>
    /// There is no defined separator: producers use commas, semicolons or spaces. Splitting on spaces
    /// unconditionally would break multi-word keywords, so a space separates only when no punctuation
    /// does.
    /// </remarks>
    private static string[] SplitKeywords(string? keywords)
    {
        if (string.IsNullOrWhiteSpace(keywords)) return [];

        char[] separators = keywords.AsSpan().IndexOfAny(',', ';') >= 0 ? [',', ';'] : [' '];
        return keywords.Split(
            separators,
            StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
    }

    /// <summary>
    /// The revision number, which the property set stores as text rather than a number.
    /// </summary>
    private static int? RevisionOf(string? revision)
        => int.TryParse(revision, NumberStyles.Integer, CultureInfo.InvariantCulture, out int value)
            ? value
            : null;

    private static OlePropertySet ReadSet(IPackage package, string streamName)
    {
        IPackagePart? part = package.GetPart(streamName);
        if (part is null || part.Length is <= 0 or > MaxStreamBytes) return OlePropertySet.Empty;

        byte[] bytes;
        try
        {
            using Stream stream = part.Open();
            using MemoryStream buffer = new((int)part.Length);
            stream.CopyTo(buffer);
            bytes = buffer.ToArray();
        }
        catch (IOException)
        {
            return OlePropertySet.Empty;
        }

        return OlePropertySet.Parse(bytes);
    }

    /// <summary>
    /// The well-known property identifiers of the summary information set.
    /// </summary>
    /// <remarks>
    /// The same numbers mean different properties in the document summary set, which is why the two
    /// are separate classes rather than one list: reading a document-summary property by a summary
    /// identifier silently returns the wrong field.
    /// </remarks>
    public static class SummaryId
    {
        /// <summary>The code page the set's byte strings are encoded in.</summary>
        public const int CodePage = 1;

        /// <summary>The document's title.</summary>
        public const int Title = 2;

        /// <summary>The document's subject.</summary>
        public const int Subject = 3;

        /// <summary>The document's author.</summary>
        public const int Author = 4;

        /// <summary>The document's keywords, as one string.</summary>
        public const int Keywords = 5;

        /// <summary>The document's comments, which Paperless maps to its description.</summary>
        public const int Comments = 6;

        /// <summary>The template the document was created from.</summary>
        public const int Template = 7;

        /// <summary>Who saved the document last.</summary>
        public const int LastAuthor = 8;

        /// <summary>The revision number, stored as text.</summary>
        public const int RevisionNumber = 9;

        /// <summary>How long the document has been edited for.</summary>
        public const int EditingTime = 10;

        /// <summary>When the document was last printed.</summary>
        public const int LastPrinted = 11;

        /// <summary>When the document was created.</summary>
        public const int Created = 12;

        /// <summary>When the document was last saved.</summary>
        public const int LastSaved = 13;

        /// <summary>The document's page count.</summary>
        public const int PageCount = 14;

        /// <summary>The document's word count.</summary>
        public const int WordCount = 15;

        /// <summary>The document's character count.</summary>
        public const int CharacterCount = 16;

        /// <summary>The application that wrote the document.</summary>
        public const int ApplicationName = 18;
    }

    /// <summary>The well-known property identifiers of the document summary information set.</summary>
    public static class DocumentSummaryId
    {
        /// <summary>The document's category.</summary>
        public const int Category = 2;

        /// <summary>The document's line count.</summary>
        public const int LineCount = 12;

        /// <summary>The document's paragraph count.</summary>
        public const int ParagraphCount = 13;

        /// <summary>The document's manager.</summary>
        public const int Manager = 14;

        /// <summary>The document's company.</summary>
        public const int Company = 15;
    }

    /// <summary>
    /// One parsed property-set stream: its properties by identifier, and its custom named ones.
    /// </summary>
    /// <remarks>
    /// A stream holds one or more <em>sections</em>, each a property dictionary of its own. The
    /// sections are merged here rather than kept apart, because the document summary stream's second
    /// section exists only to hold user-defined properties and a caller has no reason to care which
    /// section a property came from — only whether it was named or numbered.
    /// </remarks>
    private sealed class OlePropertySet
    {
        private readonly Dictionary<int, object?> _properties = [];
        private readonly Dictionary<string, object?> _named = [];

        /// <summary>An empty set, for a document with no such stream.</summary>
        public static OlePropertySet Empty { get; } = new();

        /// <summary>The user-defined properties, by the names the document gives them.</summary>
        public IReadOnlyDictionary<string, object?> NamedProperties => _named;

        /// <summary>Parses a property-set stream.</summary>
        public static OlePropertySet Parse(byte[] stream)
        {
            OlePropertySet set = new();
            ReadOnlySpan<byte> span = stream;

            if (span.Length < 28) return set;
            if (BinaryPrimitives.ReadUInt16LittleEndian(span) != ByteOrderMarker) return set;

            int sections = BinaryPrimitives.ReadInt32LittleEndian(span[24..]);
            if (sections is <= 0 or > 32) return set;
            if (28 + (20 * sections) > span.Length) return set;

            for (int i = 0; i < sections; i++)
            {
                // Each entry is a 16-byte FMTID followed by the section's offset. The FMTID says
                // which property set the section belongs to, which Paperless does not need to
                // check: the stream it came from already says that.
                int offset = BinaryPrimitives.ReadInt32LittleEndian(span[(28 + (20 * i) + 16)..]);
                if (offset < 0 || offset >= span.Length) continue;
                set.ReadSection(span, offset);
            }

            return set;
        }

        /// <summary>The property with an identifier read as text, or null when there is none.</summary>
        public string? Text(int id)
            => _properties.TryGetValue(id, out object? value) && value is string { Length: > 0 } text
                ? text
                : null;

        /// <summary>The property with an identifier read as a count.</summary>
        public int? Integer(int id)
            => _properties.TryGetValue(id, out object? value) && value is int and >= 0
                ? (int)value
                : null;

        /// <summary>The property with an identifier read as a point in time.</summary>
        public DateTimeOffset? Timestamp(int id)
            => _properties.TryGetValue(id, out object? value) && value is DateTime moment
                ? new DateTimeOffset(moment)
                : null;

        /// <summary>
        /// The property with an identifier read as an elapsed time.
        /// </summary>
        /// <remarks>
        /// Editing time is stored in a <c>FILETIME</c> — a field meant for absolute instants — so the
        /// value arrives as an interval measured from the 1601 epoch rather than as a date, which is
        /// why this cannot share the timestamp accessor.
        /// </remarks>
        public TimeSpan? Duration(int id)
            => _properties.TryGetValue(id, out object? value) && value is TimeSpan elapsed
                ? elapsed
                : null;

        private void ReadSection(ReadOnlySpan<byte> stream, int sectionStart)
        {
            ReadOnlySpan<byte> section = stream[sectionStart..];
            if (section.Length < 8) return;

            // Offsets inside a section are relative to the section's own start, so the span is
            // narrowed to the section rather than indexed from the stream.
            int sectionLength = BinaryPrimitives.ReadInt32LittleEndian(section);
            if (sectionLength >= 8 && sectionLength <= section.Length) section = section[..sectionLength];

            int count = BinaryPrimitives.ReadInt32LittleEndian(section[4..]);
            if (count <= 0) return;
            count = Math.Min(count, MaxPropertiesPerSection);
            if (8 + (8 * count) > section.Length) return;

            // The code page governs how the section's byte strings decode, and it is itself one of
            // the section's properties — so it has to be found before any string is read rather
            // than whenever the walk happens to reach it.
            Encoding encoding = EncodingOf(section, count);

            // The dictionary maps user-defined property ids to names, and likewise must be read
            // before the properties it names.
            Dictionary<int, string> names = NamesOf(section, count, encoding);

            for (int i = 0; i < count; i++)
            {
                int id = BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i))..]);
                int offset = BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i) + 4)..]);
                if (offset < 0 || offset >= section.Length) continue;

                // Identifiers 0 and 1 are the dictionary and the code page: machinery rather than
                // metadata.
                if (id is 0 or SummaryId.CodePage) continue;

                object? value = ReadValue(section[offset..], encoding);
                if (value is null) continue;

                if (names.TryGetValue(id, out string? name) && name.Length > 0) _named[name] = value;
                else _properties[id] = value;
            }
        }

        private static Encoding EncodingOf(ReadOnlySpan<byte> section, int count)
        {
            for (int i = 0; i < count; i++)
            {
                if (BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i))..]) != SummaryId.CodePage)
                    continue;

                int offset = BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i) + 4)..]);
                if (offset < 0 || offset + 6 > section.Length) break;

                // The value is a signed 16-bit integer, so UTF-8's 65001 arrives as a negative
                // number — LibreOffice writes exactly that for the documents it saves, so reading it
                // as unsigned is not an edge case.
                short raw = BinaryPrimitives.ReadInt16LittleEndian(section[(offset + 4)..]);
                int codePage = raw < 0 ? raw + 65536 : raw;

                // 1200 is not a code page a byte-string decoder can be built from: it declares that
                // the section's strings are UTF-16, which changes how their lengths are counted as
                // well as how their bytes decode.
                if (codePage == Utf16CodePage) return Encoding.Unicode;

                Encoding resolved = LegacyCodePages.Get(codePage, out bool exact);
                return exact ? resolved : LegacyCodePages.Fallback;
            }

            return LegacyCodePages.Fallback;
        }

        /// <summary>
        /// The names of the section's user-defined properties, from its dictionary property.
        /// </summary>
        /// <remarks>
        /// The dictionary is not a typed value like every other property: it is a bare count followed
        /// by id-and-name pairs, so the value reader cannot read it.
        /// </remarks>
        private static Dictionary<int, string> NamesOf(
            ReadOnlySpan<byte> section, int count, Encoding encoding)
        {
            Dictionary<int, string> names = [];

            int dictionaryOffset = -1;
            for (int i = 0; i < count; i++)
            {
                if (BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i))..]) != 0) continue;
                dictionaryOffset = BinaryPrimitives.ReadInt32LittleEndian(section[(8 + (8 * i) + 4)..]);
                break;
            }

            if (dictionaryOffset < 0 || dictionaryOffset + 4 > section.Length) return names;

            ReadOnlySpan<byte> dictionary = section[dictionaryOffset..];
            int entries = BinaryPrimitives.ReadInt32LittleEndian(dictionary);
            if (entries is <= 0 or > MaxPropertiesPerSection) return names;

            // A Unicode dictionary counts characters and pads each entry to four bytes; a byte one
            // counts bytes and pads nothing. Nothing in the entry distinguishes them, so the
            // section's code page has to decide.
            bool unicode = encoding.CodePage == Utf16CodePage;

            int position = 4;
            for (int i = 0; i < entries && position + 8 <= dictionary.Length; i++)
            {
                int id = BinaryPrimitives.ReadInt32LittleEndian(dictionary[position..]);
                int length = BinaryPrimitives.ReadInt32LittleEndian(dictionary[(position + 4)..]);
                position += 8;

                if (length < 0) break;

                int bytes = unicode ? length * 2 : length;
                if (bytes < 0 || position + bytes > dictionary.Length) break;

                names[id] = (unicode
                    ? Encoding.Unicode.GetString(dictionary.Slice(position, bytes))
                    : encoding.GetString(dictionary.Slice(position, bytes))).TrimEnd('\0');
                position += bytes;

                if (unicode && (position & 3) != 0) position += 4 - (position & 3);
            }

            return names;
        }

        /// <summary>
        /// Reads one typed value.
        /// </summary>
        /// <remarks>
        /// Only the types that carry document metadata are decoded. An unrecognised type returns null
        /// rather than a best guess, because a property read at the wrong type produces a plausible
        /// value out of unrelated bytes — worse than reporting nothing.
        /// </remarks>
        private static object? ReadValue(ReadOnlySpan<byte> value, Encoding encoding)
        {
            if (value.Length < 4) return null;

            ushort type = BinaryPrimitives.ReadUInt16LittleEndian(value);
            ReadOnlySpan<byte> body = value[4..];

            switch (type)
            {
                case VariantType.Int16 or VariantType.UInt16:
                    return body.Length >= 2
                        ? (int)BinaryPrimitives.ReadInt16LittleEndian(body)
                        : null;

                case VariantType.Int32 or VariantType.UInt32:
                    return body.Length >= 4 ? BinaryPrimitives.ReadInt32LittleEndian(body) : null;

                case VariantType.Boolean:
                    return body.Length >= 2 && BinaryPrimitives.ReadInt16LittleEndian(body) != 0;

                case VariantType.ByteString:
                    return ReadByteString(body, encoding);

                case VariantType.WideString:
                    return ReadWideString(body);

                case VariantType.FileTime:
                    return ReadFileTime(body);

                default:
                    return null;
            }
        }

        private static string? ReadByteString(ReadOnlySpan<byte> body, Encoding encoding)
        {
            if (body.Length < 4) return null;
            int length = BinaryPrimitives.ReadInt32LittleEndian(body);
            if (length <= 0 || 4 + length > body.Length) return null;

            // The count includes the terminating NUL, which is not part of the string.
            return encoding.GetString(body.Slice(4, length)).TrimEnd('\0');
        }

        private static string? ReadWideString(ReadOnlySpan<byte> body)
        {
            if (body.Length < 4) return null;
            int characters = BinaryPrimitives.ReadInt32LittleEndian(body);
            if (characters <= 0) return null;

            int bytes = characters * 2;
            if (bytes < 0 || 4 + bytes > body.Length) return null;

            return Encoding.Unicode.GetString(body.Slice(4, bytes)).TrimEnd('\0');
        }

        /// <summary>
        /// Reads a <c>FILETIME</c>, returning an instant when it plausibly is one and an interval
        /// when it is not.
        /// </summary>
        /// <remarks>
        /// The field carries both, and nothing in it says which. An elapsed time is a small count of
        /// ticks from the 1601 epoch, so a value that would land in the seventeenth century is a
        /// duration rather than a date — which is how editing time and creation date share one
        /// encoding without being confused for each other.
        /// </remarks>
        private static object? ReadFileTime(ReadOnlySpan<byte> body)
        {
            if (body.Length < 8) return null;

            long ticks = BinaryPrimitives.ReadInt64LittleEndian(body);
            if (ticks <= 0) return null;

            // A century of ticks: no real document date is this close to the epoch.
            const long CenturyInTicks = 100L * 365 * 24 * 60 * 60 * 10_000_000;
            if (ticks < CenturyInTicks) return TimeSpan.FromTicks(ticks);

            try
            {
                return DateTime.FromFileTimeUtc(ticks);
            }
            catch (ArgumentOutOfRangeException)
            {
                return null;
            }
        }
    }

    /// <summary>The <c>[MS-OLEPS]</c> variant type tags Paperless decodes.</summary>
    private static class VariantType
    {
        /// <summary>A signed 16-bit integer.</summary>
        public const ushort Int16 = 2;

        /// <summary>A signed 32-bit integer.</summary>
        public const ushort Int32 = 3;

        /// <summary>A boolean, stored in sixteen bits.</summary>
        public const ushort Boolean = 11;

        /// <summary>An unsigned 16-bit integer.</summary>
        public const ushort UInt16 = 18;

        /// <summary>An unsigned 32-bit integer.</summary>
        public const ushort UInt32 = 19;

        /// <summary>A string in the section's code page.</summary>
        public const ushort ByteString = 30;

        /// <summary>A UTF-16 string.</summary>
        public const ushort WideString = 31;

        /// <summary>A 64-bit tick count, used for both instants and durations.</summary>
        public const ushort FileTime = 64;
    }
}
