importClass(Packages.com.sun.star.uno.UnoRuntime);
importClass(Packages.com.sun.star.sheet.XSpreadsheetDocument);
importClass(Packages.com.sun.star.container.XIndexAccess);
importClass(Packages.com.sun.star.beans.XPropertySet);
importClass(Packages.com.sun.star.beans.PropertyValue);
importClass(Packages.com.sun.star.util.XModifiable);
importClass(Packages.com.sun.star.frame.XStorable);
importClass(Packages.com.sun.star.frame.XModel);
importClass(Packages.com.sun.star.uno.AnyConverter);
importClass(Packages.com.sun.star.uno.Type);

importClass(java.lang.System);

oDoc = XSCRIPTCONTEXT.getDocument();
xSDoc = UnoRuntime.queryInterface(XSpreadsheetDocument, oDoc);
xModel = UnoRuntime.queryInterface(XModel,oDoc);
xSheetsIndexAccess = UnoRuntime.queryInterface(XIndexAccess, xSDoc.getSheets());
xStorable = UnoRuntime.queryInterface(XStorable,xSDoc);
xModifiable = UnoRuntime.queryInterface(XModifiable,xSDoc);

storeProps = new Array;//PropertyValue[1];
storeProps[0] = new PropertyValue();
storeProps[0].Name = "FilterName";
storeProps[0].Value = "HTML (StarCalc)";
storeUrl = xModel.getURL();
storeUrl = storeUrl.substring(0,storeUrl.lastIndexOf('.'));

//set only one sheet visible, and store to HTML doc
for(var i=0;i<xSheetsIndexAccess.getCount();i++)
{
	setAllButOneHidden(xSheetsIndexAccess,i);
	xModifiable.setModified(false);
	xStorable.storeToURL(storeUrl+"_sheet"+(i+1)+".html", storeProps);
}

// now set all visible again
for(var i=0;i<xSheetsIndexAccess.getCount();i++)
{
	xPropSet = AnyConverter.toObject( new Type(XPropertySet), xSheetsIndexAccess.getByIndex(i));
	xPropSet.setPropertyValue("IsVisible", true);	
}

function setAllButOneHidden(xSheetsIndexAccess,vis) {
	//System.err.println("count="+xSheetsIndexAccess.getCount());
	xPropSet = AnyConverter.toObject( new Type(XPropertySet), xSheetsIndexAccess.getByIndex(vis));
	xPropSet.setPropertyValue("IsVisible", true);
	for(var i=0;i<xSheetsIndexAccess.getCount();i++)
	{
		xPropSet = AnyConverter.toObject( new Type(XPropertySet), xSheetsIndexAccess.getByIndex(i));
		if(i!=vis) {
			xPropSet.setPropertyValue("IsVisible", false);
		}
	}
} 
