using System.Globalization;
using System.Security.Cryptography;
using System.Text;
using XRay.Office.Core.Documents;

namespace XRay.Office.Rendering.Pdf;

/// <summary>
/// The document information dictionary and the XMP packet: what a PDF says about itself.
/// </summary>
/// <remarks>
/// <para>
/// Both are written from the same facts, because a reader that finds them disagreeing believes
/// neither: Title, Author (the creator), Subject (the description), Keywords, Creator (the
/// application that produced the source document, which is what LibreOffice puts there) and Producer
/// (this library). The dates are the PDF's own — when it was written — in both.
/// </para>
/// <para>
/// Strings go through <see cref="Clean"/> first. A document's metadata is whatever its author typed,
/// control characters and unpaired surrogates included, and neither a PDF string nor an XML text node
/// can carry them.
/// </para>
/// </remarks>
internal static class PdfMetadata
{
    /// <summary>The facts a file states about itself, with the empty ones already dropped.</summary>
    public sealed record Facts(
        string? Title,
        string? Author,
        string? Subject,
        IReadOnlyList<string> Keywords,
        string? Creator,
        string Producer,
        string? Language,
        DateTimeOffset Created);

    /// <summary>Collects the facts from the options and the document's metadata.</summary>
    public static Facts From(PdfRenderOptions options, DateTimeOffset created)
    {
        DocumentMetadata? metadata = options.WriteDocumentInfo ? options.Metadata : null;

        List<string> keywords = [];
        if (metadata is not null)
        {
            foreach (string keyword in metadata.Keywords)
            {
                if (Clean(keyword) is { Length: > 0 } cleaned) keywords.Add(cleaned);
            }
        }

        return new Facts(
            Title: Present(metadata?.Title),
            Author: Present(metadata?.Author),
            // The description is what an office suite shows as "Comments"; Subject is its own field and
            // is the one a PDF means by the word, falling back to the description when none is set.
            Subject: Present(metadata?.Subject) ?? Present(metadata?.Description),
            Keywords: keywords,
            Creator: Present(metadata?.GeneratorApplication),
            Producer: Present(options.Producer) ?? "X-Ray.Office",
            Language: LanguageTag(options.Language ?? metadata?.Language),
            Created: created);
    }

    /// <summary>
    /// A string with the characters a PDF or XML text cannot hold removed, and trimmed.
    /// </summary>
    public static string Clean(string? value)
    {
        if (string.IsNullOrEmpty(value)) return string.Empty;

        StringBuilder builder = new(value.Length);
        for (int i = 0; i < value.Length; i++)
        {
            char c = value[i];
            if (char.IsHighSurrogate(c) && i + 1 < value.Length && char.IsLowSurrogate(value[i + 1]))
            {
                builder.Append(c).Append(value[++i]);
            }
            else if (char.IsSurrogate(c) || c == '￾' || c == '￿')
            {
                continue;
            }
            else if (c is '\r' or '\n' or '\t')
            {
                builder.Append(' ');
            }
            else if (!char.IsControl(c))
            {
                builder.Append(c);
            }
        }

        return builder.ToString().Trim();
    }

    private static string? Present(string? value) => Clean(value) is { Length: > 0 } cleaned ? cleaned : null;

    /// <summary>
    /// A BCP 47 tag as the catalogue's <c>/Lang</c> wants it, or null when the text is not one.
    /// </summary>
    /// <remarks>
    /// Office files record a language as a locale — <c>en-US</c>, sometimes <c>en_US</c> — and
    /// occasionally as a word. Only the shapes a tag can take are accepted, so a stray
    /// <c>"English"</c> is left out rather than written into the file as though it were a code.
    /// </remarks>
    public static string? LanguageTag(string? value)
    {
        if (Clean(value).Replace('_', '-') is not { Length: > 0 } tag) return null;

        string[] parts = tag.Split('-');
        if (parts[0].Length is < 2 or > 3 || !parts[0].All(char.IsAsciiLetter)) return null;
        if (parts.Skip(1).Any(part => part.Length is < 1 or > 8 || !part.All(char.IsAsciiLetterOrDigit))) return null;

        return tag;
    }

    /// <summary>The body of the information dictionary.</summary>
    public static string InfoDictionary(Facts facts)
    {
        StringBuilder info = new("<<");
        Entry(info, "Title", facts.Title);
        Entry(info, "Author", facts.Author);
        Entry(info, "Subject", facts.Subject);
        if (facts.Keywords.Count > 0) Entry(info, "Keywords", string.Join(", ", facts.Keywords));
        Entry(info, "Creator", facts.Creator);
        Entry(info, "Producer", facts.Producer);
        info.Append("/CreationDate").Append(PdfSyntax.Date(facts.Created));
        info.Append("/ModDate").Append(PdfSyntax.Date(facts.Created));

        return info.Append(">>").ToString();

        static void Entry(StringBuilder into, string key, string? value)
        {
            if (value is not null) into.Append('/').Append(key).Append(PdfSyntax.LiteralString(value));
        }
    }

    /// <summary>
    /// A minimal, valid XMP packet carrying the same facts.
    /// </summary>
    public static byte[] Xmp(Facts facts)
    {
        string date = facts.Created.ToString("yyyy-MM-dd'T'HH:mm:sszzz", CultureInfo.InvariantCulture);

        StringBuilder xmp = new();
        xmp.Append("<?xpacket begin=\"﻿\" id=\"W5M0MpCehiHzreSzNTczkc9d\"?>\n");
        xmp.Append("<x:xmpmeta xmlns:x=\"adobe:ns:meta/\">\n");
        xmp.Append("<rdf:RDF xmlns:rdf=\"http://www.w3.org/1999/02/22-rdf-syntax-ns#\">\n");
        xmp.Append("<rdf:Description rdf:about=\"\"\n");
        xmp.Append(" xmlns:dc=\"http://purl.org/dc/elements/1.1/\"\n");
        xmp.Append(" xmlns:pdf=\"http://ns.adobe.com/pdf/1.3/\"\n");
        xmp.Append(" xmlns:xmp=\"http://ns.adobe.com/xap/1.0/\">\n");
        xmp.Append("<dc:format>application/pdf</dc:format>\n");

        if (facts.Title is not null)
            xmp.Append("<dc:title><rdf:Alt><rdf:li xml:lang=\"x-default\">").Append(Escape(facts.Title))
                .Append("</rdf:li></rdf:Alt></dc:title>\n");

        if (facts.Author is not null)
            xmp.Append("<dc:creator><rdf:Seq><rdf:li>").Append(Escape(facts.Author))
                .Append("</rdf:li></rdf:Seq></dc:creator>\n");

        if (facts.Subject is not null)
            xmp.Append("<dc:description><rdf:Alt><rdf:li xml:lang=\"x-default\">").Append(Escape(facts.Subject))
                .Append("</rdf:li></rdf:Alt></dc:description>\n");

        if (facts.Keywords.Count > 0)
        {
            xmp.Append("<dc:subject><rdf:Bag>");
            foreach (string keyword in facts.Keywords) xmp.Append("<rdf:li>").Append(Escape(keyword)).Append("</rdf:li>");
            xmp.Append("</rdf:Bag></dc:subject>\n");
        }

        if (facts.Language is not null)
            xmp.Append("<dc:language><rdf:Bag><rdf:li>").Append(Escape(facts.Language))
                .Append("</rdf:li></rdf:Bag></dc:language>\n");

        xmp.Append("<pdf:Producer>").Append(Escape(facts.Producer)).Append("</pdf:Producer>\n");
        if (facts.Keywords.Count > 0)
            xmp.Append("<pdf:Keywords>").Append(Escape(string.Join(", ", facts.Keywords))).Append("</pdf:Keywords>\n");

        if (facts.Creator is not null)
            xmp.Append("<xmp:CreatorTool>").Append(Escape(facts.Creator)).Append("</xmp:CreatorTool>\n");

        xmp.Append("<xmp:CreateDate>").Append(date).Append("</xmp:CreateDate>\n");
        xmp.Append("<xmp:ModifyDate>").Append(date).Append("</xmp:ModifyDate>\n");
        xmp.Append("<xmp:MetadataDate>").Append(date).Append("</xmp:MetadataDate>\n");
        xmp.Append("</rdf:Description>\n</rdf:RDF>\n</x:xmpmeta>\n");
        xmp.Append("<?xpacket end=\"w\"?>");

        return new UTF8Encoding(encoderShouldEmitUTF8Identifier: false).GetBytes(xmp.ToString());
    }

    /// <summary>
    /// A file identifier that is the same for the same content, so that output stays reproducible.
    /// </summary>
    public static byte[] FileId(Facts facts, int objectCount)
    {
        string seed = string.Create(
            CultureInfo.InvariantCulture,
            $"{facts.Title}|{facts.Author}|{facts.Producer}|{facts.Created:O}|{objectCount}");
        return SHA256.HashData(Encoding.UTF8.GetBytes(seed))[..16];
    }

    private static string Escape(string value) => System.Security.SecurityElement.Escape(value) ?? string.Empty;
}
