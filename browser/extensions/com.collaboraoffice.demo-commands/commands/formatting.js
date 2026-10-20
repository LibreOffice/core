// The selected parts of text, each a text with the offsets of its first and last selected
// character:
function selectedParts(selection) {
	const parts = [];
	for (const rangeElement of selection.getRangeElements()) {
		const element = rangeElement.getElement();
		const type = element.getType();
		let text;
		if (type === DocumentApp.ElementType.TEXT) {
			text = element.asText();
		} else if (type === DocumentApp.ElementType.PARAGRAPH) {
			text = element.asParagraph().editAsText();
		} else if (type === DocumentApp.ElementType.LIST_ITEM) {
			text = element.asListItem().editAsText();
		} else {
			continue;
		}
		const length = text.getText().length;
		if (length === 0) {
			continue;
		}
		if (rangeElement.isPartial()) {
			parts.push({
				text,
				start: rangeElement.getStartOffset(),
				end: rangeElement.getEndOffsetInclusive(),
			});
		} else {
			parts.push({ text, start: 0, end: length - 1 });
		}
	}
	return parts;
}

function toggleSelected(selection, getter, setter) {
	const parts = selectedParts(selection);
	if (parts.length === 0) {
		return;
	}
	const value = parts[0].text[getter](parts[0].start) !== true;
	for (const part of parts) {
		part.text[setter](part.start, part.end, value);
	}
}

var commands = {
	makeBold: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			toggleSelected(selection, 'isBold', 'setBold');
		} else {
			const cursor = doc.getCursor();
			if (cursor) cursor.setBold(!cursor.isBold());
		}
	},
	makeItalic: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			toggleSelected(selection, 'isItalic', 'setItalic');
		} else {
			const cursor = doc.getCursor();
			if (cursor) cursor.setItalic(!cursor.isItalic());
		}
	},
	makeUnderline: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			toggleSelected(selection, 'isUnderline', 'setUnderline');
		} else {
			const cursor = doc.getCursor();
			if (cursor) cursor.setUnderline(!cursor.isUnderline());
		}
	},
	clearFormatting: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			for (const part of selectedParts(selection)) {
				part.text.setBold(part.start, part.end, false);
				part.text.setItalic(part.start, part.end, false);
				part.text.setUnderline(part.start, part.end, false);
			}
		} else {
			doc.getCursor()?.setBold(false).setItalic(false).setUnderline(false);
		}
	},
};
