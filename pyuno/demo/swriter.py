
# bootstrap uno component context 	
import uno
import unohelper


# a UNO struct later needed to create a document
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.text.TextContentAnchorType import AS_CHARACTER
from com.sun.star.awt import Size


def insertTextIntoCell( table, cellName, text, color ):
    tableText = table.getCellByName( cellName )
    cursor = tableText.createTextCursor()
    cursor.setPropertyValue( "CharColor", color )
    tableText.setString( text )

localContext = uno.getComponentContext()
unohelper.addComponentsToContext(  localContext , localContext,
				   ("connectr","uuresolver","remotebridge","brdgfctr"),
				   "com.sun.star.loader.SharedLibrary" )
				   
resolver = localContext.ServiceManager.createInstanceWithContext(
				"com.sun.star.bridge.UnoUrlResolver", localContext )

smgr = resolver.resolve( "uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager" )
remoteContext = smgr.getPropertyValue( "DefaultContext" )

#remoteContext = resolver.resolve( "uno:socket,host=localhost,port=2002;urp;StarOffice.ComponentContext" )
#smgr = remoteContext.ServiceManager

desktop = smgr.createInstanceWithContext( "com.sun.star.frame.Desktop",remoteContext)

# open a writer document
doc = desktop.loadComponentFromURL( "private:factory/swriter","_blank", 0, () )

text = doc.Text
cursor = text.createTextCursor()
text.insertString( cursor, "The first line in the newly created text document.\n", 0 )
text.insertString( cursor, "Now we are in the second line\n" , 0 )

# create a text table
table = doc.createInstance( "com.sun.star.text.TextTable" )

# with 4 rows and 4 columns
table.initialize( 4,4)

text.insertTextContent( cursor, table, 0 )
rows = table.Rows

table.setPropertyValue( "BackTransparent", uno.Bool(0) )
table.setPropertyValue( "BackColor", 13421823 )
row = rows.getByIndex(0)
row.setPropertyValue( "BackTransparent", uno.Bool(0) )
row.setPropertyValue( "BackColor", 6710932 )

textColor = 16777215

insertTextIntoCell( table, "A1", "FirstColumn", textColor )
insertTextIntoCell( table, "B1", "SecondColumn", textColor )
insertTextIntoCell( table, "C1", "ThirdColumn", textColor )
insertTextIntoCell( table, "D1", "SUM", textColor )

values = ( (22.5,21.5,121.5),
	   (5615.3,615.3,-615.3),
	   (-2315.7,315.7,415.7) )
table.getCellByName("A2").setValue(22.5)
table.getCellByName("B2").setValue(5615.3)
table.getCellByName("C2").setValue(-2315.7)
table.getCellByName("D2").setFormula("sum <A2:C2>")

table.getCellByName("A3").setValue(21.5)
table.getCellByName("B3").setValue(615.3)
table.getCellByName("C3").setValue(-315.7)
table.getCellByName("D3").setFormula("sum <A3:C3>")

table.getCellByName("A4").setValue(121.5)
table.getCellByName("B4").setValue(-615.3)
table.getCellByName("C4").setValue(415.7)
table.getCellByName("D4").setFormula("sum <A4:C4>")


cursor.setPropertyValue( "CharColor", 255 )
cursor.setPropertyValue( "CharShadowed", uno.Bool(1) )

text.insertControlCharacter( cursor, PARAGRAPH_BREAK, 0 )
text.insertString( cursor, " This is a colored Text - blue with shadow\n" , 0 )
text.insertControlCharacter( cursor, PARAGRAPH_BREAK, 0 )

textFrame = doc.createInstance( "com.sun.star.text.TextFrame" )
textFrame.setSize( Size(15000,400))
textFrame.setPropertyValue( "AnchorType" , AS_CHARACTER )


text.insertTextContent( cursor, textFrame, 0 )

textInTextFrame = textFrame.getText()
cursorInTextFrame = textInTextFrame.createTextCursor()
textInTextFrame.insertString( cursorInTextFrame, "The first line in the newly created text frame.", 0 )
textInTextFrame.insertString( cursorInTextFrame, "\nWith this second line the height of the rame raises.",0)
text.insertControlCharacter( cursor, PARAGRAPH_BREAK, 0 )

cursor.setPropertyValue( "CharColor", 65536 )
cursor.setPropertyValue( "CharShadowed", uno.Bool(0) )

text.insertString( cursor, " That's all for now !!" , 0 )

