/*************************************************************************
 *
 *  $RCSfile: pastedlg.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-21 16:47:47 $
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

#ifdef SVX_DLLIMPLEMENTATION
#undef SVX_DLLIMPLEMENTATION
#endif

#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif

#include <pastedlg.hxx>
#include <svtools/svmedit.hxx>
#include <svtools/insdlg.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/group.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include "svuidlg.hrc"
#include <sot/formats.hxx>
#include <sot/stg.hxx>
#include <svtools/sores.hxx>

#include "dialmgr.hxx"

SvPasteObjectDialog::SvPasteObjectDialog( Window* pParent )

    : ModalDialog( pParent, ResId( MD_PASTE_OBJECT, DIALOG_MGR() ) ),
    aFtSource( this, ResId( FT_SOURCE ) ),
    aFtObjectSource( this, ResId( FT_OBJECT_SOURCE ) ),
    aRbPaste( this, ResId( RB_PASTE ) ),
    aRbPasteLink( this, ResId( RB_PASTE_LINK ) ),
    aLbInsertList( this, ResId( LB_INSERT_LIST ) ),
    aCbDisplayAsIcon( this, ResId( CB_DISPLAY_AS_ICON ) ),
    aPbChangeIcon( this, ResId( PB_CHANGE_ICON ) ),
    aFlChoice( this, ResId( FL_CHOICE ) ),
    aOKButton1( this, ResId( 1 ) ),
    aCancelButton1( this, ResId( 1 ) ),
    aHelpButton1( this, ResId( 1 ) ),
    aSObject( ResId( S_OBJECT ) )
{
    FreeResource();
    SetHelpId( HID_PASTE_DLG );
    SetUniqueId( HID_PASTE_DLG );

    Font aFont = aFtObjectSource.GetFont();
    aFont.SetWeight( WEIGHT_LIGHT );
    aFtObjectSource.SetFont( aFont );
    aOKButton1.Disable();

    ObjectLB().SetSelectHdl( LINK( this, SvPasteObjectDialog, SelectHdl ) );
    ObjectLB().SetDoubleClickHdl( LINK( this, SvPasteObjectDialog, DoubleClickHdl ) );
    SetDefault();
}

void SvPasteObjectDialog::SelectObject()
{
    if ( aLbInsertList.GetEntryCount() &&
         !aRbPaste.IsVisible() && !aRbPasteLink.IsVisible() )
    {
        aLbInsertList.SelectEntryPos(0);
        SelectHdl( &aLbInsertList );
    }
}

IMPL_LINK( SvPasteObjectDialog, SelectHdl, ListBox *, pListBox )
{
    if ( !aOKButton1.IsEnabled() )
        aOKButton1.Enable();
    return 0;
}

IMPL_LINK_INLINE_START( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )
{
    EndDialog( RET_OK );
    return 0;
}
IMPL_LINK_INLINE_END( SvPasteObjectDialog, DoubleClickHdl, ListBox *, pListBox )

void SvPasteObjectDialog::SetDefault()
{
    bLink   = FALSE;
    nAspect = ::com::sun::star::embed::Aspects::MSOLE_CONTENT;
}

SvPasteObjectDialog::~SvPasteObjectDialog()
{
    void * pStr = aSupplementTable.First();
    while( pStr )
    {
        delete (String *)pStr;
        pStr = aSupplementTable.Next();
    }
}

/*************************************************************************
|*    SvPasteObjectDialog::Insert()
|*
|*    Beschreibung
|*    Ersterstellung    MM 14.06.94
|*    Letzte Aenderung  KA 16.03.2001
*************************************************************************/
void SvPasteObjectDialog::Insert( SotFormatStringId nFormat, const String& rFormatName )
{
    String * pStr = new String( rFormatName );
    if( !aSupplementTable.Insert( nFormat, pStr ) )
        delete pStr;
}

ULONG SvPasteObjectDialog::GetFormat( const TransferableDataHelper& rHelper,
                                      const DataFlavorExVector* pFormats,
                                      const TransferableObjectDescriptor* )
{
    //TODO/LATER: why is the Descriptor never used?!
    TransferableObjectDescriptor aDesc;
    if( rHelper.HasFormat( SOT_FORMATSTR_ID_OBJECTDESCRIPTOR ) )
        ((TransferableDataHelper&)rHelper).GetTransferableObjectDescriptor(
                                SOT_FORMATSTR_ID_OBJECTDESCRIPTOR, aDesc );
    if ( !pFormats )
        pFormats = &rHelper.GetDataFlavorExVector();

    //Dialogbox erzeugen und fuellen
    String aSourceName, aTypeName;
    ULONG nSelFormat = 0;
    SvGlobalName aEmptyNm;

    ObjectLB().SetUpdateMode( FALSE );

    DataFlavorExVector::iterator aIter( ((DataFlavorExVector&)*pFormats).begin() ),
                                 aEnd( ((DataFlavorExVector&)*pFormats).end() );
    while( aIter != aEnd )
    {
        ::com::sun::star::datatransfer::DataFlavor aFlavor( *aIter );
        SotFormatStringId nFormat = (*aIter++).mnSotId;

        String* pName = (String*) aSupplementTable.Get( nFormat );
        String aName;

#ifdef WNT
/*
        if( !pName &&
            ( nFormat == SOT_FORMATSTR_ID_EMBED_SOURCE_OLE || nFormat == SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE ) )
        {
            BOOL IsClipboardObject_Impl( SotDataObject * );
            if( IsClipboardObject_Impl( pDataObj ) )
            {
                IDataObject * pDO = NULL;
                OleGetClipboard( &pDO );
                if( pDO )
                {
                    FORMATETC fe;
                    STGMEDIUM stm;
                    (fe).cfFormat=RegisterClipboardFormat( "Object Descriptor" );
                    (fe).dwAspect=DVASPECT_CONTENT;
                    (fe).ptd=NULL;
                    (fe).tymed=TYMED_HGLOBAL;
                    (fe).lindex=-1;

                    if (SUCCEEDED(pDO->GetData(&fe, &stm)))
                    {
                        LPOBJECTDESCRIPTOR pOD=(LPOBJECTDESCRIPTOR)GlobalLock(stm.hGlobal);
                        if( pOD->dwFullUserTypeName )
                        {
                            OLECHAR * pN = (OLECHAR *)(((BYTE *)pOD) + pOD->dwFullUserTypeName);
                            aName.Append( pN );
                            pName = &aName;
                            // set format to ole object
                            nFormat = SOT_FORMATSTR_ID_EMBED_SOURCE_OLE;
                        }
                        if( pOD->dwSrcOfCopy )
                        {
                            OLECHAR * pN = (OLECHAR *)(((BYTE *)pOD) + pOD->dwSrcOfCopy);
                            aSourceName.Append( *pN++ );
                        }
                        else
                            aSourceName = String( ResId( STR_UNKNOWN_SOURCE, SOAPP->GetResMgr() ) );
                        GlobalUnlock(stm.hGlobal);
                        ReleaseStgMedium(&stm);
                    }
                }
            }
        }
*/
#endif

        // if there is an "Embed Source" or and "Embedded Object" on the
        // Clipboard we read the Description and the Source of this object
        // from an accompanied "Object Descriptor" format on the clipboard
        // Remember: these formats mostly appear together on the clipboard
        if ( !pName )
        {
            SvPasteObjectHelper::GetEmbeddedName(rHelper,aName,aSourceName,nFormat);
            if ( aName.Len() )
                pName = &aName;
        }


        if( pName )
        {
            aName = *pName;

            if( SOT_FORMATSTR_ID_EMBED_SOURCE == nFormat )
            {
                if( aDesc.maClassName != aEmptyNm )                                {
                    aSourceName = aDesc.maDisplayName;

                    if( aDesc.maClassName == aObjClassName )
                        aName = aObjName;
                    else
                        aName = aTypeName = aDesc.maTypeName;
                }
            }
            else if( SOT_FORMATSTR_ID_LINK_SOURCE == nFormat )
            {
                PasteLink().Enable();
                continue;
            }
            else if( !aName.Len() )
                aName = SvPasteObjectHelper::GetSotFormatUIName( nFormat );

            if( LISTBOX_ENTRY_NOTFOUND == ObjectLB().GetEntryPos( aName ) )
                ObjectLB().SetEntryData(
                    ObjectLB().InsertEntry( aName ), (void*) nFormat );
        }
    }

    if( !aTypeName.Len() && !aSourceName.Len() )
    {
        if( aDesc.maClassName != aEmptyNm )
        {
            aSourceName = aDesc.maDisplayName;
            aTypeName = aDesc.maTypeName;
        }

        if( !aTypeName.Len() && !aSourceName.Len() )
            // global resource from svtools (former so3 resource)
            aSourceName = String( ResId( STR_UNKNOWN_SOURCE ) );
    }

    ObjectLB().SetUpdateMode( TRUE );
    SelectObject();

    if( aSourceName.Len() )
    {
        if( aTypeName.Len() )
            aTypeName += '\n';

        aTypeName += aSourceName;
        aTypeName.ConvertLineEnd();
    }

    ObjectSource().SetText( aTypeName );

    SetDefault();

    if( Dialog::Execute() == RET_OK )
    {
        bLink = PasteLink().IsChecked();

        if( AsIconBox().IsChecked() )
            nAspect = com::sun::star::embed::Aspects::MSOLE_ICON;

        nSelFormat  = (ULONG)ObjectLB().GetEntryData( ObjectLB().GetSelectEntryPos() );
    }

    return nSelFormat;
}

void SvPasteObjectDialog::SetObjName( const SvGlobalName & rClass, const String & rObjName )
{
    aObjClassName = rClass;
    aObjName = rObjName;
}

