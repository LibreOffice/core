var commands = {
	makeBold: function () {
		var cursor = cool.getActiveDocument().getCursor().uno;
		var bold = uno.idl.com.sun.star.awt.FontWeight.BOLD;
		var normal = uno.idl.com.sun.star.awt.FontWeight.NORMAL;
		var current = cursor.getPropertyValue('CharWeight');
		cursor.setPropertyValue('CharWeight', current === bold ? normal : bold);
	},
	makeItalic: function () {
		var cursor = cool.getActiveDocument().getCursor().uno;
		var italic = uno.idl.com.sun.star.awt.FontSlant.ITALIC;
		var none = uno.idl.com.sun.star.awt.FontSlant.NONE;
		var current = cursor.getPropertyValue('CharPosture');
		cursor.setPropertyValue('CharPosture', current === italic ? none : italic);
	},
	makeUnderline: function () {
		var cursor = cool.getActiveDocument().getCursor().uno;
		var single = uno.idl.com.sun.star.awt.FontUnderline.SINGLE;
		var none = uno.idl.com.sun.star.awt.FontUnderline.NONE;
		var current = cursor.getPropertyValue('CharUnderline');
		cursor.setPropertyValue('CharUnderline', current === single ? none : single);
	},
	clearFormatting: function () {
		var cursor = cool.getActiveDocument().getCursor().uno;
		cursor.setPropertyValue('CharWeight', uno.idl.com.sun.star.awt.FontWeight.NORMAL);
		cursor.setPropertyValue('CharPosture', uno.idl.com.sun.star.awt.FontSlant.NONE);
		cursor.setPropertyValue('CharUnderline', uno.idl.com.sun.star.awt.FontUnderline.NONE);
	},
};
