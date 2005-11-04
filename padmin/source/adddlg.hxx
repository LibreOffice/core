/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: adddlg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-04 15:40:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _PAD_ADDDLG_HXX_
#define _PAD_ADDDLG_HXX_

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _PSP_PRINTERINFOMANAGER_HXX_
#include <psprint/printerinfomanager.hxx>
#endif
#ifndef _PAD_HELPER_HXX_
#include <helper.hxx>
#endif
#ifndef _PAD_TITLECTRL_HXX_
#include <titlectrl.hxx>
#endif

namespace padmin
{

class AddPrinterDialog;

namespace DeviceKind { enum type { Printer, Fax, Pdf }; }

class APTabPage : public TabPage
{
    String              m_aTitle;
protected:
    AddPrinterDialog*   m_pParent;
public:
    APTabPage( AddPrinterDialog* pParent, const ResId& rResId );

    // returns false if information is incomplete or invalid
    virtual bool check() = 0;
    virtual void fill( ::psp::PrinterInfo& rInfo ) = 0;
    const String& getTitle() const { return m_aTitle; }
};

class APChooseDevicePage : public APTabPage
{
    RadioButton             m_aPrinterBtn;
    RadioButton             m_aFaxBtn;
    RadioButton             m_aPDFBtn;
    RadioButton             m_aOldBtn;
    FixedText               m_aOverTxt;
public:
    APChooseDevicePage( AddPrinterDialog* pParent );
    ~APChooseDevicePage();

    bool isPrinter() { return m_aPrinterBtn.IsChecked(); }
    bool isFax() { return m_aFaxBtn.IsChecked(); }
    bool isPDF() { return m_aPDFBtn.IsChecked(); }
    bool isOld() { return m_aOldBtn.IsChecked(); }


    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APChooseDriverPage : public APTabPage
{
    FixedText               m_aDriverTxt;
    DelListBox              m_aDriverBox;
    PushButton              m_aAddBtn;
    PushButton              m_aRemBtn;

    String                  m_aRemStr;
    String                  m_aLastPrinterName;

    DECL_LINK( ClickBtnHdl, PushButton* );
    DECL_LINK( DelPressedHdl, ListBox* );

    void updateDrivers();
public:
    APChooseDriverPage( AddPrinterDialog* pParent );
    ~APChooseDriverPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APNamePage : public APTabPage
{
    FixedText               m_aNameTxt;
    Edit                    m_aNameEdt;
    CheckBox                m_aDefaultBox;
    CheckBox                m_aFaxSwallowBox;
public:
    APNamePage( AddPrinterDialog* pParent, const String& rInitName, DeviceKind::type eKind );
    ~APNamePage();

    bool isDefault() { return m_aDefaultBox.IsChecked(); }
    bool isFaxSwallow() { return m_aFaxSwallowBox.IsChecked(); }

    void setText( const String& rText ) { m_aNameEdt.SetText( rText ); }

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );
};

class APCommandPage : public APTabPage
{
    FixedText               m_aCommandTxt;
    ComboBox                m_aCommandBox;
    PushButton              m_aHelpBtn;
    String                  m_aHelpTxt;
    FixedText               m_aPdfDirTxt;
    Edit                    m_aPdfDirEdt;
    PushButton              m_aPdfDirBtn;

    DeviceKind::type        m_eKind;

    DECL_LINK( ClickBtnHdl, PushButton* );
    DECL_LINK( ModifyHdl, ComboBox* );
public:

    APCommandPage( AddPrinterDialog* pParent, DeviceKind::type eKind );
    ~APCommandPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    String getPdfDir() { return m_aPdfDirEdt.GetText(); }
};

class APOldPrinterPage : public APTabPage
{
    FixedText                           m_aOldPrinterTxt;
    MultiListBox                        m_aOldPrinterBox;
    PushButton                          m_aSelectAllBtn;

    ::std::list< ::psp::PrinterInfo >   m_aOldPrinters;

    DECL_LINK( ClickBtnHdl, PushButton* );
public:
    APOldPrinterPage( AddPrinterDialog* pParent );
    ~APOldPrinterPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    void addOldPrinters();
};

class APFaxDriverPage : public APTabPage
{
    FixedText               m_aFaxTxt;
    RadioButton             m_aDefBtn;
    RadioButton             m_aSelectBtn;
public:
    APFaxDriverPage( AddPrinterDialog* pParent );
    ~APFaxDriverPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    bool isDefault() { return m_aDefBtn.IsChecked(); }
};

class APPdfDriverPage : public APTabPage
{
    FixedText               m_aPdfTxt;
    RadioButton             m_aDefBtn;
    RadioButton             m_aDistBtn;
    RadioButton             m_aSelectBtn;
public:
    APPdfDriverPage( AddPrinterDialog* pParent );
    ~APPdfDriverPage();

    virtual bool check();
    virtual void fill( ::psp::PrinterInfo& rInfo );

    bool isDefault() { return m_aDefBtn.IsChecked(); }
    bool isDist() { return m_aDistBtn.IsChecked(); }
};

class AddPrinterDialog : public ModalDialog
{
    CancelButton            m_aCancelPB;
    PushButton              m_aPrevPB;
    PushButton              m_aNextPB;
    OKButton                m_aFinishPB;
    FixedLine               m_aLine;
    TitleImage              m_aTitleImage;

    ::psp::PrinterInfo      m_aPrinter;

    APTabPage*              m_pCurrentPage;

    APChooseDevicePage*     m_pChooseDevicePage;
    APCommandPage*          m_pCommandPage;
    APChooseDriverPage*     m_pChooseDriverPage;
    APNamePage*             m_pNamePage;
    APOldPrinterPage*       m_pOldPrinterPage;
    APFaxDriverPage*        m_pFaxDriverPage;
    APChooseDriverPage*     m_pFaxSelectDriverPage;
    APNamePage*             m_pFaxNamePage;
    APCommandPage*          m_pFaxCommandPage;
    APPdfDriverPage*        m_pPdfDriverPage;
    APChooseDriverPage*     m_pPdfSelectDriverPage;
    APNamePage*             m_pPdfNamePage;
    APCommandPage*          m_pPdfCommandPage;

    DECL_LINK( ClickBtnHdl, PushButton* );

    void advance();
    void back();
    void addPrinter();

    void updateSettings();
    virtual void DataChanged( const DataChangedEvent& rEv );

public:
    AddPrinterDialog( Window* pParent );
    ~AddPrinterDialog();

    static String uniquePrinterName( const String& rString );
    static String getOldPrinterLocation();

    void enableNext( bool bEnable ) { m_aNextPB.Enable( bEnable ); }
};

} // namespace

#endif
