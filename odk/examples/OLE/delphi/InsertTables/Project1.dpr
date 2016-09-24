program Project1;

uses
  Forms,
  SampleUI in 'SampleUI.pas' {OKBottomDlg},
  SampleCode in 'SampleCode.pas';

{$R *.RES}

begin
  Application.Initialize;
  Application.CreateForm(TOKBottomDlg, OKBottomDlg);
  Application.Run;
end.
