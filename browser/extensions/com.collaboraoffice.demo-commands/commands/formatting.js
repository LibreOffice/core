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
			const range = doc.getCursor().uno;
			var bold = uno.idl.com.sun.star.awt.FontWeight.BOLD;
			var normal = uno.idl.com.sun.star.awt.FontWeight.NORMAL;
			var value = range.getPropertyValue('CharWeight') === bold ? normal : bold;
			range.setPropertyValue('CharWeight', value);
		}
	},
	makeItalic: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			toggleSelected(selection, 'isItalic', 'setItalic');
		} else {
			const range = doc.getCursor().uno;
			var italic = uno.idl.com.sun.star.awt.FontSlant.ITALIC;
			var none = uno.idl.com.sun.star.awt.FontSlant.NONE;
			var value = range.getPropertyValue('CharPosture') === italic ? none : italic;
			range.setPropertyValue('CharPosture', value);
		}
	},
	makeUnderline: function () {
		const doc = DocumentApp.getActiveDocument();
		const selection = doc.getSelection();
		if (selection) {
			toggleSelected(selection, 'isUnderline', 'setUnderline');
		} else {
			const range = doc.getCursor().uno;
			var single = uno.idl.com.sun.star.awt.FontUnderline.SINGLE;
			var none = uno.idl.com.sun.star.awt.FontUnderline.NONE;
			var value = range.getPropertyValue('CharUnderline') === single ? none : single;
			range.setPropertyValue('CharUnderline', value);
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
			const range = doc.getCursor().uno;
			range.setPropertyValue('CharWeight', uno.idl.com.sun.star.awt.FontWeight.NORMAL);
			range.setPropertyValue('CharPosture', uno.idl.com.sun.star.awt.FontSlant.NONE);
			range.setPropertyValue('CharUnderline', uno.idl.com.sun.star.awt.FontUnderline.NONE);
		}
	},
};
