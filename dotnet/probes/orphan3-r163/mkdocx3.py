import mkdocx2, zipfile
def write(path, body, mb=1080):
    doc = mkdocx2.DOC.replace('w:bottom="1080"', f'w:bottom="{mb}"')
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", mkdocx2.CT)
        z.writestr("_rels/.rels", mkdocx2.RELS)
        z.writestr("word/_rels/document.xml.rels", mkdocx2.DRELS)
        z.writestr("word/settings.xml", mkdocx2.SETTINGS)
        z.writestr("word/styles.xml", mkdocx2.STYLES)
        z.writestr("word/document.xml", doc % {"body": body})
    return path
