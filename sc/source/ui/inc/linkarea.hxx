/*************************************************************************
 *
 *  $RCSfile: linkarea.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 20:17:50 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

    ScDocShell*         pSourceShell;
    SfxObjectShellRef   aSourceRef;
    ::com::sun::star::uno::Reference< ::com::sun::star::embed::XEmbeddedObject > xSourceObject;

    DECL_LINK( FileHdl, ComboBox* );
    DECL_LINK( BrowseHdl, PushButton* );
    DECL_LINK( RangeHdl, MultiListBox* );
    DECL_LINK( ReloadHdl, CheckBox* );
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

