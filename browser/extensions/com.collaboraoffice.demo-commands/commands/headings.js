function paragraphOf(element) {
	if (
		element.getType() === DocumentApp.ElementType.TEXT ||
		element.getType() === DocumentApp.ElementType.INLINE_IMAGE
	) {
		element = element.getParent();
		if (!element) {
			return null;
		}
	}
	const type = element.getType();
	if (type === DocumentApp.ElementType.PARAGRAPH) {
		return element.asParagraph();
	}
	if (type === DocumentApp.ElementType.LIST_ITEM) {
		return element.asListItem();
	}
	return null;
}

function setHeading(heading) {
	const doc = DocumentApp.getActiveDocument();
	const selection = doc.getSelection();
	const cursor = doc.getCursor();
	const elements = selection
		? selection.getRangeElements().map((rangeElement) => rangeElement.getElement())
		: cursor ? [cursor.getElement()] : [];
	for (const element of elements) {
		const paragraph = paragraphOf(element);
		if (paragraph) {
			paragraph.setHeading(heading);
		}
	}
}

var commands = {
	heading1: function () {
		setHeading(DocumentApp.ParagraphHeading.HEADING1);
	},
	heading2: function () {
		setHeading(DocumentApp.ParagraphHeading.HEADING2);
	},
	heading3: function () {
		setHeading(DocumentApp.ParagraphHeading.HEADING3);
	},
};
