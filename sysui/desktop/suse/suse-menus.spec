Summary: OpenOffice.org desktop integration
Name: openoffice-suse-menus
Version: 2.0
Release: 1
Group: Office
Copyright: commercial
AutoReqProv: no
%define _unpackaged_files_terminate_build 0
%description 
OpenOffice.org desktop integration

%files
%defattr(-, root, root)
%dir /opt/gnome2/share/application-registry
%dir /opt/gnome2/share/applications
%dir /opt/gnome2/share/mime-info
%dir /opt/gnome2/share/icons
%dir /opt/gnome2/share/icons/gnome
%dir /opt/gnome2/share/icons/gnome/16x16
%dir /opt/gnome2/share/icons/gnome/16x16/apps
%dir /opt/gnome2/share/icons/gnome/16x16/mimetypes
%dir /opt/gnome2/share/icons/gnome/22x22
%dir /opt/gnome2/share/icons/gnome/22x22/apps
%dir /opt/gnome2/share/icons/gnome/22x22/mimetypes
%dir /opt/gnome2/share/icons/gnome/32x32
%dir /opt/gnome2/share/icons/gnome/32x32/apps
%dir /opt/gnome2/share/icons/gnome/32x32/mimetypes
%dir /opt/gnome2/share/icons/gnome/48x48
%dir /opt/gnome2/share/icons/gnome/48x48/apps
%dir /opt/gnome2/share/icons/gnome/48x48/mimetypes
%dir /opt/kde3/share/icons
%dir /opt/kde3/share/applnk
%dir /opt/kde3/share/icons/hicolor
%dir /opt/kde3/share/icons/hicolor/16x16
%dir /opt/kde3/share/icons/hicolor/16x16/apps
%dir /opt/kde3/share/icons/hicolor/16x16/mimetypes
%dir /opt/kde3/share/icons/hicolor/22x22
%dir /opt/kde3/share/icons/hicolor/22x22/apps
%dir /opt/kde3/share/icons/hicolor/22x22/mimetypes
%dir /opt/kde3/share/icons/hicolor/32x32
%dir /opt/kde3/share/icons/hicolor/32x32/apps
%dir /opt/kde3/share/icons/hicolor/32x32/mimetypes
%dir /opt/kde3/share/icons/hicolor/48x48
%dir /opt/kde3/share/icons/hicolor/48x48/apps
%dir /opt/kde3/share/icons/hicolor/48x48/mimetypes
%dir /opt/kde3/share/icons/locolor
%dir /opt/kde3/share/icons/locolor/16x16
%dir /opt/kde3/share/icons/locolor/16x16/apps
%dir /opt/kde3/share/icons/locolor/16x16/mimetypes
%dir /opt/kde3/share/icons/locolor/22x22
%dir /opt/kde3/share/icons/locolor/22x22/apps
%dir /opt/kde3/share/icons/locolor/22x22/mimetypes
%dir /opt/kde3/share/icons/locolor/32x32
%dir /opt/kde3/share/icons/locolor/32x32/apps
%dir /opt/kde3/share/icons/locolor/32x32/mimetypes
%dir /opt/kde3/share/mimelnk
/opt/gnome2/share/application-registry/%PREFIX.applications
/opt/gnome2/share/applications/%PREFIX-writer.desktop
/opt/gnome2/share/applications/%PREFIX-calc.desktop
/opt/gnome2/share/applications/%PREFIX-draw.desktop
/opt/gnome2/share/applications/%PREFIX-impress.desktop
/opt/gnome2/share/applications/%PREFIX-math.desktop
/opt/gnome2/share/applications/%PREFIX-printeradmin.desktop
/opt/kde3/share/applnk/Office/%PREFIX-writer.desktop
/opt/kde3/share/applnk/Office/%PREFIX-calc.desktop
/opt/kde3/share/applnk/Office/%PREFIX-draw.desktop
/opt/kde3/share/applnk/Office/%PREFIX-impress.desktop
/opt/kde3/share/applnk/Office/%PREFIX-math.desktop
/opt/kde3/share/applnk/Office/%PREFIX-printeradmin.desktop
/opt/gnome2/share/mime-info/%PREFIX.keys
/opt/kde3/share/mimelnk/application/%PREFIX-text.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-text-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-spreadsheet.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-spreadsheet-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-drawing.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-drawing-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-presentation.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-presentation-template.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-master-document.desktop
/opt/kde3/share/mimelnk/application/%PREFIX-formula.desktop
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-writer.png
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-calc.png
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-draw.png
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-impress.png
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-math.png
/opt/gnome2/share/icons/gnome/16x16/apps/%PREFIX-printeradmin.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-text.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-text-template.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-spreadsheet.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-spreadsheet-template.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-drawing.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-drawing-template.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-presentation.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-presentation-template.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-master-document.png
/opt/gnome2/share/icons/gnome/16x16/mimetypes/%PREFIX-formula.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-writer.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-calc.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-draw.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-impress.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-math.png
/opt/gnome2/share/icons/gnome/22x22/apps/%PREFIX-printeradmin.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-text.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-text-template.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-spreadsheet.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-spreadsheet-template.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-drawing.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-drawing-template.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-presentation.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-presentation-template.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-master-document.png
/opt/gnome2/share/icons/gnome/22x22/mimetypes/%PREFIX-formula.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-writer.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-calc.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-draw.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-impress.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-math.png
/opt/gnome2/share/icons/gnome/32x32/apps/%PREFIX-printeradmin.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-text.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-text-template.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-spreadsheet.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-spreadsheet-template.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-drawing.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-drawing-template.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-presentation.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-presentation-template.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-master-document.png
/opt/gnome2/share/icons/gnome/32x32/mimetypes/%PREFIX-formula.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-writer.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-calc.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-draw.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-impress.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-math.png
/opt/gnome2/share/icons/gnome/48x48/apps/%PREFIX-printeradmin.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-text.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-text-template.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-spreadsheet.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-spreadsheet-template.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-drawing.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-drawing-template.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-presentation.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-presentation-template.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-master-document.png
/opt/gnome2/share/icons/gnome/48x48/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-writer.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-calc.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-draw.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-impress.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-math.png
/opt/kde3/share/icons/hicolor/16x16/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/hicolor/16x16/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-writer.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-calc.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-draw.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-impress.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-math.png
/opt/kde3/share/icons/hicolor/22x22/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/hicolor/22x22/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-writer.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-calc.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-draw.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-impress.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-math.png
/opt/kde3/share/icons/hicolor/32x32/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/hicolor/32x32/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-writer.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-calc.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-draw.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-impress.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-math.png
/opt/kde3/share/icons/hicolor/48x48/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/hicolor/48x48/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-writer.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-calc.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-draw.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-impress.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-math.png
/opt/kde3/share/icons/locolor/16x16/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/locolor/16x16/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-writer.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-calc.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-draw.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-impress.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-math.png
/opt/kde3/share/icons/locolor/22x22/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/locolor/22x22/mimetypes/%PREFIX-formula.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-writer.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-calc.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-draw.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-impress.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-math.png
/opt/kde3/share/icons/locolor/32x32/apps/%PREFIX-printeradmin.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-text.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-text-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-spreadsheet.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-spreadsheet-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-drawing.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-drawing-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-presentation.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-presentation-template.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-master-document.png
/opt/kde3/share/icons/locolor/32x32/mimetypes/%PREFIX-formula.png
