var commands = {
	makeBold: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		var bold = uno.idl.com.sun.star.awt.FontWeight.BOLD;
		var normal = uno.idl.com.sun.star.awt.FontWeight.NORMAL;
		var current = viewCursor.getPropertyValue('CharWeight');
		viewCursor.setPropertyValue('CharWeight', current === bold ? normal : bold);
	},
	makeItalic: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		var italic = uno.idl.com.sun.star.awt.FontSlant.ITALIC;
		var none = uno.idl.com.sun.star.awt.FontSlant.NONE;
		var current = viewCursor.getPropertyValue('CharPosture');
		viewCursor.setPropertyValue('CharPosture', current === italic ? none : italic);
	},
	makeUnderline: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		var single = uno.idl.com.sun.star.awt.FontUnderline.SINGLE;
		var none = uno.idl.com.sun.star.awt.FontUnderline.NONE;
		var current = viewCursor.getPropertyValue('CharUnderline');
		viewCursor.setPropertyValue('CharUnderline', current === single ? none : single);
	},
	clearFormatting: function () {
		var desktop = uno.idl.com.sun.star.frame.Desktop.create(uno.componentContext);
		var viewCursor = desktop.getCurrentFrame().getController().getViewCursor();
		viewCursor.setPropertyValue('CharWeight', uno.idl.com.sun.star.awt.FontWeight.NORMAL);
		viewCursor.setPropertyValue('CharPosture', uno.idl.com.sun.star.awt.FontSlant.NONE);
		viewCursor.setPropertyValue('CharUnderline', uno.idl.com.sun.star.awt.FontUnderline.NONE);
	},
};
