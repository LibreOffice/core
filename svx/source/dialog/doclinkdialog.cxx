/*************************************************************************
 *
 *  $RCSfile: doclinkdialog.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-04 12:04:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SVX_DOCLINKDIALOG_HXX_
#include "doclinkdialog.hxx"
#endif
#ifndef _SVX_DOCLINKDIALOG_HRC_
#include "doclinkdialog.hrc"
#endif
#include "dialogs.hrc"
#include "svxids.hrc"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef SVTOOLS_FILENOTATION_HXX_
#include <svtools/filenotation.hxx>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#include "dialmgr.hxx"
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _FILEDLGHELPER_HXX
#include <sfx2/filedlghelper.hxx>
#endif
#ifndef _SFX_DOCFILT_HACK_HXX
#include <sfx2/docfilt.hxx>
#endif
//......................................................................
namespace svx
{
//......................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::ucb;
    using namespace ::svt;

    //==================================================================
    //= ODocumentLinkDialog
    //==================================================================
    //------------------------------------------------------------------
    ODocumentLinkDialog::ODocumentLinkDialog( Window* _pParent, sal_Bool _bCreateNew )
        :ModalDialog( _pParent, SVX_RES(DLG_DOCUMENTLINK) )
        ,m_aNameLabel       (this, ResId(FT_NAME))
        ,m_aName            (this, ResId(ET_NAME))
        ,m_aURLLabel        (this, ResId(FT_URL))
        ,m_aURL             (this, ResId(CMB_URL))
        ,m_aBrowseFile      (this, ResId(PB_BROWSEFILE))
        ,m_aBottomLine      (this, ResId(FL_BOTTOM))
        ,m_aOK              (this, ResId(BTN_OK))
        ,m_aCancel          (this, ResId(BTN_CANCEL))
        ,m_aHelp            (this, ResId(BTN_HELP))
        ,m_bCreatingNew(_bCreateNew)
    {
        String sText = String( ResId( m_bCreatingNew ? STR_NEW_LINK : STR_EDIT_LINK ) );
        SetText(sText);

        FreeResource();

        String sTemp = String::CreateFromAscii("*.odb");
        m_aURL.SetFilter(sTemp);

        m_aName.SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_aURL.SetModifyHdl( LINK(this, ODocumentLinkDialog, OnTextModified) );
        m_aBrowseFile.SetClickHdl( LINK(this, ODocumentLinkDialog, OnBrowseFile) );
        m_aOK.SetClickHdl( LINK(this, ODocumentLinkDialog, OnOk) );

        m_aURL.SetDropDownLineCount(10);

        validate();

        //  m_aURL.SetHelpId( HID_DOCLINKEDIT_URL );
        m_aURL.SetDropDownLineCount( 5 );
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::set( const String& _rName, const String& _rURL )
    {
        m_aName.SetText(_rName);
        m_aURL.SetText(_rURL);
        validate();
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::get( String& _rName, String& _rURL ) const
    {
        _rName = m_aName.GetText();
        _rURL = m_aURL.GetText();
    }

    //------------------------------------------------------------------
    void ODocumentLinkDialog::validate( )
    {

        m_aOK.Enable( (0 != m_aName.GetText().Len()) && ( 0 != m_aURL.GetText().Len() ) );
    }

    //------------------------------------------------------------------
    IMPL_LINK( ODocumentLinkDialog, OnOk, void*, NOINTERESTEDIN )
    {
        // get the current URL
        ::rtl::OUString sURL = m_aURL.GetText();
        OFileNotation aTransformer(sURL);
        sURL = aTransformer.get(OFileNotation::N_URL);

        // check for the existence of the selected file
        sal_Bool bFileExists = sal_False;
        try
        {
            ::ucb::Content aFile(sURL, Reference< XCommandEnvironment >());
            if (aFile.isDocument())
                bFileExists = sal_True;
        }
        catch(Exception&)
        {
        }

        if (!bFileExists)
        {
            String sMsg = String(SVX_RES(STR_LINKEDDOC_DOESNOTEXIST));
            sMsg.SearchAndReplaceAscii("$file$", m_aURL.GetText());
            ErrorBox aError(this, WB_OK , sMsg);
            aError.Execute();
            return 0L;
        } // if (!bFileExists)
        String sCurrentText = m_aName.GetText();
        if ( m_aNameValidator.IsSet() )
        {
            if ( !m_aNameValidator.Call( &sCurrentText ) )
            {
                String sMsg = String(SVX_RES(STR_NAME_CONFLICT));
                sMsg.SearchAndReplaceAscii("$file$", sCurrentText);
                InfoBox aError(this, sMsg);
                aError.Execute();

                m_aName.SetSelection(Selection(0,sCurrentText.Len()));
                m_aName.GrabFocus();
                return 0L;
            }
        }

        EndDialog(RET_OK);
        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK( ODocumentLinkDialog, OnBrowseFile, void*, NOINTERESTEDIN )
    {
        ::sfx2::FileDialogHelper aFileDlg(WB_3DLOOK | WB_STDMODAL | WB_OPEN);
        static const String s_sDatabaseType = String::CreateFromAscii("StarOffice XML (Base)");
        const SfxFilter* pFilter = SfxFilter::GetFilterByName( s_sDatabaseType);
        if ( pFilter )
        {
            aFileDlg.AddFilter(pFilter->GetFilterName(),pFilter->GetDefaultExtension());
            aFileDlg.SetCurrentFilter(pFilter->GetFilterName());
        }

        String sPath = m_aURL.GetText();
        if (sPath.Len())
        {
            OFileNotation aTransformer( sPath, OFileNotation::N_SYSTEM );
            aFileDlg.SetDisplayDirectory( aTransformer.get( OFileNotation::N_URL ) );
        }

        if (0 != aFileDlg.Execute())
            return 0L;

        if (0 == m_aName.GetText().Len())
        {   // default the name to the base of the chosen URL
            INetURLObject aParser;

            aParser.SetSmartProtocol(INET_PROT_FILE);
            aParser.SetSmartURL(aFileDlg.GetPath());

            m_aName.SetText(aParser.getBase(INetURLObject::LAST_SEGMENT, true, INetURLObject::DECODE_WITH_CHARSET));

            m_aName.SetSelection(Selection(0,m_aName.GetText().Len()));
            m_aName.GrabFocus();
        }
        else
            m_aURL.GrabFocus();

        // get the path in system notation
        OFileNotation aTransformer(aFileDlg.GetPath(), OFileNotation::N_URL);
        m_aURL.SetText(aTransformer.get(OFileNotation::N_SYSTEM));

        validate();
        return 0L;
    }

    //------------------------------------------------------------------
    IMPL_LINK( ODocumentLinkDialog, OnTextModified, Control*, _pWhich )
    {
        validate( );
        return 0L;
    }

//......................................................................
}   // namespace svx
//......................................................................

