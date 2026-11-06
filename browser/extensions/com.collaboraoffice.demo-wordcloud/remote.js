remote.documentText = function documentText(useSelection) {
    const doc = DocumentApp.getActiveDocument();
    if (useSelection) {
        return doc.getSelection().getText();
    }
    return doc.getBody().getText();
};

remote.insertImage = function insertImage(bytes, widthCm, heightCm) {
    const doc = DocumentApp.getActiveDocument();
    // With a selection, the image goes right after the end of it:
    let position = doc.getCursor();
    if (!position) {
        const elements = doc.getSelection().getRangeElements();
        const last = elements[elements.length - 1];
        const element = last.getElement();
        if (last.isPartial()) {
            position = doc.newPosition(element, last.getEndOffsetInclusive() + 1);
        } else if (element.getType() === DocumentApp.ElementType.TEXT) {
            position = doc.newPosition(element, element.asText().getText().length);
        } else if (element.getNumChildren) {
            position = doc.newPosition(element, element.getNumChildren());
        } else {
            const parent = element.getParent();
            position = doc.newPosition(parent, parent.getChildIndex(element) + 1);
        }
    }
    const pixels = (cm) => Math.round(cm / 2.54 * 96);
    position.insertInlineImage(Utilities.newBlob(bytes, 'image/svg+xml', 'wordcloud.svg'))
        .setWidth(pixels(widthCm)).setHeight(pixels(heightCm));
};
