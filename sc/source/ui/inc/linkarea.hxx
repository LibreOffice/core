/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: linkarea.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:47:58 $
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

#ifndef SC_LINKAREA_HXX
#define SC_LINKAREA_HXX

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#endif

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _STDCTRL_HXX
#include <svtools/stdctrl.hxx>
#endif
#ifndef _SVTOOLS_INETTBC_HXX
#include <svtools/inettbc.hxx>
#endif

//REMOVE    #ifndef _EMBOBJ_HXX //autogen
//REMOVE    #include <so3/embobj.hxx>
//REMOVE    #endif

//REMOVE    #ifndef SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    #define SO2_DECL_SVEMBEDDEDOBJECT_DEFINED
//REMOVE    SO2_DECL_REF(SvEmbeddedObject)
//REMOVE    #endif

namespace sfx2 { class DocumentInserter; }
namespace sfx2 { class FileDialogHelper; }

class ScDocShell;

//------------------------------------------------------------------------

class ScLinkedAreaDlg : public ModalDialog
{
private:
    FixedLine           aFlLocation;
    SvtURLBox           aCbUrl;
    PushButton          aBtnBrowse;
    FixedInfo           aTxtHint;
    FixedText           aFtRanges;
    MultiListBox        aLbRanges;
    CheckBox            aBtnReload;
    NumericField        aNfDelay;
    FixedText           aFtSeconds;
    OKButton            aBtnOk;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ScDocShell*             pSourceShell;
    sfx2::DocumentInserter* pDocInserter;

    SfxObjectShellRef   aSourceRef;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > xSourceObject;

    DECL_LINK( FileHdl, ComboBox* );
    DECL_LINK( BrowseHdl, PushButton* );
    DECL_LINK( RangeHdl, MultiListBox* );
    DECL_LINK( ReloadHdl, CheckBox* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

    void                UpdateSourceRanges();
    void                UpdateEnable();
    void                LoadDocument( const String& rFile, const String& rFilter,
                                        const String& rOptions );

public:
            ScLinkedAreaDlg( Window* pParent );
            ~ScLinkedAreaDlg();

    void            InitFromOldLink( const String& rFile, const String& rFilter,
                                        const String& rOptions, const String& rSource,
                                        ULONG nRefresh );

    virtual short   Execute();          // overwritten to set dialog parent

    String          GetURL();
    String          GetFilter();        // may be empty
    String          GetOptions();       // filter options
    String          GetSource();        // separated by ";"
    ULONG           GetRefresh();       // 0 if disabled
};

#endif

