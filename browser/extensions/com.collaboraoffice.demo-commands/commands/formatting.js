// The text ranges that a command works on, which are the selected ones when there is a selection
// and the cursor otherwise:
function targetRanges() {
	var doc = cool.getActiveDocument();
	var selection = doc.getSelection();
	if (!selection) return [doc.getCursor().uno];
	var ranges = [];
	for (var i = 0; i < selection.uno.getCount(); ++i) ranges.push(selection.uno.getByIndex(i));
	return ranges;
}

var commands = {
	makeBold: function () {
		var ranges = targetRanges();
		var bold = uno.idl.com.sun.star.awt.FontWeight.BOLD;
		var normal = uno.idl.com.sun.star.awt.FontWeight.NORMAL;
		var value = ranges[0].getPropertyValue('CharWeight') === bold ? normal : bold;
		ranges.forEach(function (range) { range.setPropertyValue('CharWeight', value); });
	},
	makeItalic: function () {
		var ranges = targetRanges();
		var italic = uno.idl.com.sun.star.awt.FontSlant.ITALIC;
		var none = uno.idl.com.sun.star.awt.FontSlant.NONE;
		var value = ranges[0].getPropertyValue('CharPosture') === italic ? none : italic;
		ranges.forEach(function (range) { range.setPropertyValue('CharPosture', value); });
	},
	makeUnderline: function () {
		var ranges = targetRanges();
		var single = uno.idl.com.sun.star.awt.FontUnderline.SINGLE;
		var none = uno.idl.com.sun.star.awt.FontUnderline.NONE;
		var value = ranges[0].getPropertyValue('CharUnderline') === single ? none : single;
		ranges.forEach(function (range) { range.setPropertyValue('CharUnderline', value); });
	},
	clearFormatting: function () {
		targetRanges().forEach(function (range) {
			range.setPropertyValue('CharWeight', uno.idl.com.sun.star.awt.FontWeight.NORMAL);
			range.setPropertyValue('CharPosture', uno.idl.com.sun.star.awt.FontSlant.NONE);
			range.setPropertyValue('CharUnderline', uno.idl.com.sun.star.awt.FontUnderline.NONE);
		});
	},
};
