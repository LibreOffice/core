def SetCellColor(x, y, color):
    """Sets the background of the cell at (x,y) (zero-based column and row
       indices, for example (2,3) == C4) on the first sheet and
       returns the contents of the cell as a string.
    """
    #get the doc from the scripting context which is made available to all scripts
    desktop = XSCRIPTCONTEXT.getDesktop()
    model = desktop.getCurrentComponent()
    #check whether there's already an opened document
    if not hasattr(model, "Sheets"):
        return ""
    sheet = model.Sheets.Sheet1
    cell = sheet.getCellByPosition(x, y)
    cell.CellBackColor = color
    return cell.String
