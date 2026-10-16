using System.IO.Compression;
using System.Text;

namespace XRay.Office.Rendering.Tests;

/// <summary>
/// Office documents written on the fly, large enough that processing them takes measurable time.
/// </summary>
/// <remarks>
/// The cancellation tests need an input that is slow on purpose, and a committed corpus file cannot be:
/// it has to stay small. These are built in memory from a few kilobytes of XML templates repeated, which
/// is also what makes them honest about what they test — a hostile document is mostly repetition.
/// </remarks>
internal static class GeneratedDocuments
{
    private const string Sentence =
        "The quick brown fox jumps over the lazy dog while the committee reviews the quarterly figures "
        + "for the northern region, which have been revised twice since the last meeting. ";

    private const string ContentTypes =
        """
        <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
        <Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">
          <Default Extension="rels" ContentType="application/vnd.openxmlformats-package.relationships+xml"/>
          <Default Extension="xml" ContentType="application/xml"/>
          <Override PartName="/word/document.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.document.main+xml"/>
          <Override PartName="/word/styles.xml" ContentType="application/vnd.openxmlformats-officedocument.wordprocessingml.styles+xml"/>
        </Types>
        """;

    private const string RootRelationships =
        """
        <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
        <Relationships xmlns="http://schemas.openxmlformats.org/package/2006/relationships">
          <Relationship Id="rId1" Type="http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument" Target="word/document.xml"/>
        </Relationships>
        """;

    private const string Styles =
        """
        <?xml version="1.0" encoding="UTF-8" standalone="yes"?>
        <w:styles xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
          <w:docDefaults><w:rPrDefault><w:rPr><w:sz w:val="22"/></w:rPr></w:rPrDefault></w:docDefaults>
          <w:style w:type="paragraph" w:default="1" w:styleId="Normal"><w:name w:val="Normal"/></w:style>
          <w:style w:type="paragraph" w:styleId="Heading1"><w:name w:val="heading 1"/><w:basedOn w:val="Normal"/>
            <w:pPr><w:keepNext/><w:outlineLvl w:val="0"/></w:pPr><w:rPr><w:b/><w:sz w:val="32"/></w:rPr></w:style>
          <w:style w:type="paragraph" w:styleId="Heading2"><w:name w:val="heading 2"/><w:basedOn w:val="Normal"/>
            <w:pPr><w:keepNext/><w:outlineLvl w:val="1"/></w:pPr><w:rPr><w:b/><w:sz w:val="26"/></w:rPr></w:style>
        </w:styles>
        """;

    /// <summary>A word-processing document of <paramref name="paragraphs"/> paragraphs of ordinary prose.</summary>
    public static byte[] Docx(int paragraphs, string? extraBody = null)
    {
        StringBuilder body = new();
        for (int i = 0; i < paragraphs; i++)
        {
            body.Append("<w:p><w:r><w:t xml:space=\"preserve\">")
                .Append(i).Append(". ")
                .Append(Sentence).Append(Sentence).Append(Sentence)
                .Append("</w:t></w:r></w:p>");
        }

        return DocxFromBody(body + (extraBody ?? string.Empty), null);
    }

    /// <summary>Wraps a body in the parts a minimal package needs.</summary>
    public static byte[] DocxFromBody(string body, string? documentRelationships, string? core = null)
    {
        string document =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<w:document xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" "
            + "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><w:body>"
            + body
            + "<w:sectPr><w:pgSz w:w=\"11906\" w:h=\"16838\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" "
            + "w:bottom=\"1440\" w:left=\"1440\" w:header=\"708\" w:footer=\"708\" w:gutter=\"0\"/></w:sectPr>"
            + "</w:body></w:document>";

        string rels =
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
            + "<Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
            + "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/styles\" Target=\"styles.xml\"/>"
            + documentRelationships
            + "</Relationships>";

        string rootRels = core is null
            ? RootRelationships
            : RootRelationships.Replace(
                "</Relationships>",
                "<Relationship Id=\"rId2\" Type=\"http://schemas.openxmlformats.org/package/2006/relationships/metadata/core-properties\" Target=\"docProps/core.xml\"/></Relationships>");

        string types = core is null
            ? ContentTypes
            : ContentTypes.Replace(
                "</Types>",
                "<Override PartName=\"/docProps/core.xml\" ContentType=\"application/vnd.openxmlformats-package.core-properties+xml\"/></Types>");

        using MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml", types);
            Write(archive, "_rels/.rels", rootRels);
            Write(archive, "word/_rels/document.xml.rels", rels);
            Write(archive, "word/styles.xml", Styles);
            Write(archive, "word/document.xml", document);
            if (core is not null) Write(archive, "docProps/core.xml", core);
        }

        return result.ToArray();
    }

    /// <summary>A workbook of one sheet with <paramref name="rows"/> rows of <paramref name="columns"/> cells.</summary>
    public static byte[] Xlsx(int rows, int columns, string sheetName = "Data")
    {
        StringBuilder sheet = new();
        sheet.Append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>");
        sheet.Append("<worksheet xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\"><sheetData>");
        for (int r = 1; r <= rows; r++)
        {
            sheet.Append("<row r=\"").Append(r).Append("\">");
            for (int c = 0; c < columns; c++)
            {
                string reference = ColumnName(c) + r;
                if (c % 2 == 0)
                {
                    sheet.Append("<c r=\"").Append(reference).Append("\"><v>").Append((r * 31) + c).Append("</v></c>");
                }
                else
                {
                    sheet.Append("<c r=\"").Append(reference).Append("\" t=\"inlineStr\"><is><t>row ")
                        .Append(r).Append(" column ").Append(c).Append("</t></is></c>");
                }
            }

            sheet.Append("</row>");
        }

        sheet.Append("</sheetData></worksheet>");

        using MemoryStream result = new();
        using (ZipArchive archive = new(result, ZipArchiveMode.Create, leaveOpen: true))
        {
            Write(archive, "[Content_Types].xml",
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Types xmlns=\"http://schemas.openxmlformats.org/package/2006/content-types\">"
                + "<Default Extension=\"rels\" ContentType=\"application/vnd.openxmlformats-package.relationships+xml\"/>"
                + "<Default Extension=\"xml\" ContentType=\"application/xml\"/>"
                + "<Override PartName=\"/xl/workbook.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet.main+xml\"/>"
                + "<Override PartName=\"/xl/worksheets/sheet1.xml\" ContentType=\"application/vnd.openxmlformats-officedocument.spreadsheetml.worksheet+xml\"/>"
                + "</Types>");
            Write(archive, "_rels/.rels",
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                + "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/officeDocument\" Target=\"xl/workbook.xml\"/></Relationships>");
            Write(archive, "xl/workbook.xml",
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><workbook xmlns=\"http://schemas.openxmlformats.org/spreadsheetml/2006/main\" "
                + "xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\"><sheets>"
                + $"<sheet name=\"{sheetName}\" sheetId=\"1\" r:id=\"rId1\"/></sheets></workbook>");
            Write(archive, "xl/_rels/workbook.xml.rels",
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?><Relationships xmlns=\"http://schemas.openxmlformats.org/package/2006/relationships\">"
                + "<Relationship Id=\"rId1\" Type=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships/worksheet\" Target=\"worksheets/sheet1.xml\"/></Relationships>");
            Write(archive, "xl/worksheets/sheet1.xml", sheet.ToString());
        }

        return result.ToArray();
    }

    private static string ColumnName(int index)
    {
        StringBuilder name = new();
        for (int n = index + 1; n > 0; n = (n - 1) / 26) name.Insert(0, (char)('A' + ((n - 1) % 26)));
        return name.ToString();
    }

    internal static void Write(ZipArchive archive, string name, string content)
    {
        using Stream entry = archive.CreateEntry(name).Open();
        entry.Write(Encoding.UTF8.GetBytes(content));
    }
}
