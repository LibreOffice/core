unit SampleUI;

interface

uses Windows, SysUtils, Classes, Graphics, Forms, Controls, StdCtrls,
  Buttons, ExtCtrls, SampleCode, ComCtrls;

type
  TOKBottomDlg = class(TForm)
    Bevel1: TBevel;
    Button1: TButton;
    Button2: TButton;
    Button3: TButton;
    Button4: TButton;
    Edit1: TEdit;
    Label1: TLabel;
    Edit2: TEdit;
    Label2: TLabel;
    Button5: TButton;
    Button6: TButton;
    Edit3: TEdit;
    Label3: TLabel;
    Label4: TLabel;
    Label6: TLabel;
    Edit6: TEdit;
    Bevel2: TBevel;
    Bevel3: TBevel;
    Bevel4: TBevel;
    StatusBar1: TStatusBar;
    Edit4: TEdit;
    Label7: TLabel;
    procedure OnConnect(Sender: TObject);
    procedure OnDisconnect(Sender: TObject);
    procedure OnCreateDocument(Sender: TObject);
    procedure OnInsertTable(Sender: TObject);
    procedure OnGetDatabasePointer(Sender: TObject);
    procedure OnGetCellContent(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  OKBottomDlg: TOKBottomDlg;
  Sample : TSampleCode;
implementation

{$R *.DFM}

procedure TOKBottomDlg.OnConnect(Sender: TObject);
begin
    StatusBar1.SimpleText := 'Connection to StarOffice ...';
    Sample := TSampleCode.Create();
    if Sample.Connect() then
    begin
        Button1.Enabled := false;
        Button2.Enabled := true;
        Button3.Enabled := true;
        Button4.Enabled := false;
        Button5.Enabled := false;
        Button6.Enabled := false;
    end;
    StatusBar1.SimpleText := 'Ready';
end;

procedure TOKBottomDlg.OnDisconnect(Sender: TObject);
begin
    StatusBar1.SimpleText := 'Disconnection from StarOffice ...';
    Sample.Disconnect();
    Button1.Enabled := true;
    Button2.Enabled := false;
    Button3.Enabled := false;
    Button4.Enabled := false;
    Button5.Enabled := false;
    Button6.Enabled := false;
    StatusBar1.SimpleText := 'Ready';
end;

procedure TOKBottomDlg.OnCreateDocument(Sender: TObject);
begin
    StatusBar1.SimpleText := 'Creating new text document ...';
    try
        if Sample.CreateDocument(false) then
        begin
            Button4.Enabled := true;
            Button5.Enabled := true;
            Button6.Enabled := true;
        end;
        StatusBar1.SimpleText := 'Ready';
    except
        StatusBar1.SimpleText := 'Error';
    end;
end;

procedure TOKBottomDlg.OnInsertTable(Sender: TObject);
begin
    try
        StatusBar1.SimpleText := 'Inserting Table ...';
        Sample.InsertTable(Edit2.Text, Edit1.Text);
        StatusBar1.SimpleText := 'Ready';
    except
        StatusBar1.SimpleText := 'Error';
    end;
end;

procedure TOKBottomDlg.OnGetDatabasePointer(Sender: TObject);
var
    res : String;
begin
    try
        StatusBar1.SimpleText := 'Getting database pointer ...';
        res := Sample.getDatabasePointer(Edit4.Text, Edit3.Text);
        Application.MessageBox(PChar('the pointer: ' + res), PChar('Result'), ID_OK);
        StatusBar1.SimpleText := 'Ready';
    except
        StatusBar1.SimpleText := 'Error';
    end;
end;

procedure TOKBottomDlg.OnGetCellContent(Sender: TObject);
var
    res : String;
begin
    try
        StatusBar1.SimpleText := 'Getting cell content ...';
        res := Sample.getCellContent(Edit6.Text);
        Application.MessageBox(PChar('the content: ' + res), PChar('Result'), ID_OK);
        StatusBar1.SimpleText := 'Ready';
    except
        StatusBar1.SimpleText := 'Error';
    end;
end;

end.
