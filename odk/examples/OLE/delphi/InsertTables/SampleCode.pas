unit SampleCode;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  StdCtrls, ComObj, Variants;

  type
    TSampleCode = class

    function Connect() : boolean;
    procedure Disconnect();

    function CreateDocument(bReadOnly : boolean) : boolean;

    procedure InsertTable(sTableName : String; dbPointer : String);

    procedure InsertDatabaseTable(
        oDoc : Variant;
        sTableName : String;
        oCursor : Variant;
        iRows : Integer;
        iColumns : Integer;
        dbPointer : String );
    function CreateTextTable(
        oDoc : Variant;
        oCursor : Variant;
        sName : String;
        iRow : Integer;
        iColumn : Integer) : Variant;
    function getCellContent(
        sBookmarkName : String ) : Variant;
    function getDatabasePointer(
        sTableName : String;
        sCellname : String ) : String;
    procedure InsertBookmark(
        oDoc : Variant;
        oTextCursor : Variant;
        sBookmarkName : String );
    function CreateBookmarkName(
        sTableName : String;
        sCellName : String;
        sDatabasepointer : String ) : String;
    procedure ChangeCellContent(
        oDoc : Variant;
        sTableName : String;
        sCellName : String;
        dValue : Double );
    function GetBookmarkFromDBPointer(
        oDoc : Variant;
        sBookmarkName : String) : Variant;
    function GetBookmarkFromAdress(
        oDoc : Variant;
        sTableName : String;
        sCellAdress : String) : Variant;
    function JumpToBookmark(
        oBookmark : Variant) : Variant;
    function CreateUniqueTablename(oDoc : Variant) : String;

  private
   StarOffice : Variant;
   Document : Variant;

    { Private-Deklarationen }
  public
    { Public-Deklarationen }
  end;

implementation

{ Insert a table texttable and insert in each cell a Bookmark with the address
  of the cell and database pointer
}

function TSampleCode.Connect() : boolean;
begin
    if  VarIsEmpty(StarOffice) then
        StarOffice := CreateOleObject('com.sun.star.ServiceManager');

    Connect := not (VarIsEmpty(StarOffice) or VarIsNull(StarOffice));
end;

procedure TSampleCode.Disconnect();
begin
    StarOffice := Unassigned;
end;

function TSampleCode.CreateDocument(bReadOnly : boolean) : boolean;
var
    StarDesktop : Variant;
    LoadParams : Variant;
    CoreReflection : Variant;
    PropertyValue : Variant;
begin
   StarDesktop := StarOffice.createInstance('com.sun.star.frame.Desktop');

   if (bReadOnly) then begin
        LoadParams := VarArrayCreate([0, 0], varVariant);
        CoreReflection := StarOffice.createInstance('com.sun.star.reflection.CoreReflection');

        CoreReflection
                .forName('com.sun.star.beans.PropertyValue')
                .createObject(PropertyValue);

        PropertyValue.Name := 'ReadOnly';
        PropertyValue.Value := true;

        LoadParams[0] := PropertyValue;
   end
   else
        LoadParams := VarArrayCreate([0, -1], varVariant);

   Document := StarDesktop.LoadComponentFromURL( 'private:factory/swriter', '_blank', 0,  LoadParams);

   CreateDocument := not (VarIsEmpty(Document) or VarIsNull(Document));
end;


function TSampleCode.getCellContent(
    sBookmarkName : String ) : Variant;
var
    oBookmark : Variant;
    oTextCursor : Variant;
begin
    oBookmark := GetBookmarkFromDBPointer( Document, sBookmarkName );
    oTextCursor := JumpToBookmark( oBookmark );

    getCellContent := oTextCursor.Cell.Value;

end;


function TSampleCode.getDatabasePointer(
    sTableName : String;
    sCellname : String ) : String;
var
    oBookmark : Variant;
    sBookmarkName : String;
    iPos : Integer;
begin
    oBookmark := GetBookmarkFromAdress( Document, sTableName, sCellName );

    sBookmarkName := oBookmark.getName();

    iPos := Pos('/%', sBookmarkName);
    while Pos('/%', sBookmarkName) > 0 do
    begin
        iPos := Pos('/%', sBookmarkName);
        sBookmarkName[iPos] := '%';
    end;

    Delete( sBookmarkName, 1, iPos+1);
    getDatabasePointer := sBookmarkName;
end;


procedure TSampleCode.InsertTable(sTableName : String; dbPointer : String);
var
   oCursor : Variant;
begin
   { create a cursor object on the current position in the document }
   oCursor := Document.Text.CreateTextCursor();

   { Create for each table a unique database name }
   if (sTableName = '') then
        sTableName := createUniqueTablename(Document);

   InsertDatabaseTable( Document, sTableName, oCursor, 4, 2, dbPointer );

   ChangeCellContent( Document, sTableName, 'B2', 1.12 );
end;

procedure TSampleCode.InsertDatabaseTable(
    oDoc : Variant;
    sTableName : String;
    oCursor : Variant;
    iRows : Integer;
    iColumns : Integer;
    dbPointer : String);
var
    oTable : Variant;
    sCellnames : Variant;
    iCellcounter : Integer;
    oCellCursor : Variant;
    oTextCursor : Variant;
    sCellName : String;
begin
    oTable := CreateTextTable( oDoc, oCursor, sTableName, iRows, iColumns );
    sCellnames := oTable.getCellNames();

    For iCellcounter := VarArrayLowBound( sCellnames, 1) to VarArrayHighBound(sCellnames, 1) do
    begin
        sCellName := sCellnames[iCellcounter];

    	oCellCursor := oTable.getCellByName(sCellName);
    	oCellCursor.Value := iCellcounter;
    	oTextCursor := oCellCursor.getEnd();
        InsertBookmark(
                oDoc,
                oTextCursor,
                createBookmarkName(sTableName, sCellName, dbPointer));
    end;
end;

{

' Change the content of a cell
}

procedure TSampleCode.ChangeCellContent(
        oDoc : Variant;
        sTableName : String;
        sCellName : String;
        dValue : Double );
var
    oBookmark : Variant;
    oTextCursor : Variant;
    sBookmarkName : String;
begin
    oBookmark := GetBookmarkFromAdress( oDoc, sTableName, sCellName );
    oTextCursor := JumpToBookmark( oBookmark );
    oTextCursor.Cell.Value := dValue;

    { create a new bookmark for the new number }
    sBookmarkName := oBookmark.getName();
    oBookmark.dispose();
    InsertBookmark( oDoc, oTextCursor, sBookmarkName );
end;


{ ' Jump to Bookmark and return for this position the cursor }

function TSampleCode.JumpToBookmark(
        oBookmark : Variant) : Variant;

begin
	JumpToBookmark := oBookmark.Anchor.Text.createTextCursorByRange(
                oBookmark.Anchor );
end;


{ ' Create a Texttable on a Textdocument }
function TSampleCode.CreateTextTable(
        oDoc : Variant;
        oCursor : Variant;
        sName : String;
        iRow : Integer;
        iColumn : Integer) : Variant;
var
    ret : Variant;
begin
    ret := oDoc.createInstance( 'com.sun.star.text.TextTable' );

    ret.setName( sName );
    ret.initialize( iRow, iColumn );
    oDoc.Text.InsertTextContent( oCursor, ret, False );

    CreateTextTable := ret;
end;


{ 'create a unique name for the Texttables }
function TSampleCode.CreateUniqueTablename(oDoc : Variant) : String;
var
    iHighestNumber : Integer;
    sTableNames : Variant;
    iTableCounter : Integer;
    sTableName : String;
    iTableNumber : Integer;
    i : Integer;
begin
    sTableNames := oDoc.getTextTables.getElementNames();
    iHighestNumber := 0;
    For iTableCounter := VarArrayLowBound(sTableNames, 1) to VarArrayHighBound(sTableNames, 1) do
    begin
    	sTableName := sTableNames[iTableCounter];
        i := Pos( '$$', sTableName );
    	iTableNumber := strtoint( Copy(sTableName, i + 2, Length( sTableName ) - i - 1 ) );

    	If iTableNumber > iHighestNumber then
    		iHighestNumber := iTableNumber;
    end;
    createUniqueTablename := 'DBTable$$' + inttostr(iHighestNumber + 1);
end;


{' Insert a Bookmark on the cursor }
procedure TSampleCode.InsertBookmark(
        oDoc : Variant;
        oTextCursor : Variant;
        sBookmarkName : String);
var
    oBookmarkInst : Variant;
begin
    oBookmarkInst := oDoc.createInstance('com.sun.star.text.Bookmark');

    oBookmarkInst.Name := sBookmarkName;
    oTextCursor.gotoStart( true );
    oTextCursor.text.InsertTextContent( oTextCursor, oBookmarkInst, true );
end;


function TSampleCode.CreateBookmarkName(
        sTableName : String;
        sCellName : String;
        sDatabasepointer : String ) : String;
begin
    createBookmarkName := '//' + sTableName + '/%' + sCellName + '/%' + sDatabasePointer + ':' + sCellName;
end;

{ ' Returns the Bookmark the Tablename and Cellname }
function TSampleCode.GetBookmarkFromAdress(
        oDoc : Variant;
        sTableName : String;
        sCellAdress : String) : Variant;
var
    sTableAddress : String;
    iTableNameLength : Integer;
    sBookNames : Variant;
    iBookCounter : Integer;
begin
    sTableAddress := '//' + sTableName + '/%' + sCellAdress;
    iTableNameLength := Length( sTableAddress );

    sBookNames := oDoc.Bookmarks.getElementNames;

    for iBookCounter := VarArrayLowBound(sBookNames, 1) to VarArrayHighBound(sBookNames, 1) do
    begin
    	If sTableAddress = Copy( sBookNames[iBookCounter], 1, iTableNameLength) then
        begin
    		GetBookmarkFromAdress := oDoc.Bookmarks.getByName(sBookNames[iBookCounter]);
    		exit;
        end;
    end;
end;

{ ' Returns the Bookmark the Tablename and Cellname }
function TSampleCode.GetBookmarkFromDBPointer(
        oDoc : Variant;
        sBookmarkName : String) : Variant;
var
    sBookNames : Variant;
    iBookCounter : Integer;
begin
    sBookNames := oDoc.Bookmarks.getElementNames;

    for iBookCounter := VarArrayLowBound(sBookNames, 1) to VarArrayHighBound(sBookNames, 1) do
    begin
    	If Pos(sBookmarkName, sBookNames[iBookCounter]) = (1 + Length(sBookNames[iBookCounter]) - Length(sBookmarkName)) then
        begin
    		GetBookmarkFromDBPointer := oDoc.Bookmarks.getByName(sBookNames[iBookCounter]);
    		exit;
        end;
    end;
end;

end.


