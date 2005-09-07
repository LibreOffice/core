/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QuerySaveDocument.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:35:43 $
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
#include "QuerySaveDocument.hxx"

#include "sfx.hrc"
#include "sfxresid.hxx"
#include "sfxuno.hxx"
#include "doc.hrc"
#ifndef _MSGBOX_HXX //autogen
#include <vcl/msgbox.hxx>
#endif
// -----------------------------------------------------------------------------
short ExecuteQuerySaveDocument(Window* _pParent,const String& _rTitle)
{
    String aText( SfxResId( STR_QUERY_SAVE_DOCUMENT ) );
    aText.SearchAndReplace( DEFINE_CONST_UNICODE( "$(DOC)" ),
                            _rTitle );
    QueryBox aQBox( _pParent, WB_YES_NO_CANCEL | WB_DEF_YES, aText );
    aQBox.SetButtonText( BUTTONID_NO, SfxResId( STR_NOSAVEANDCLOSE ) );
    aQBox.SetButtonText( BUTTONID_YES, SfxResId( STR_SAVEDOC ) );
    return aQBox.Execute();
}
// -----------------------------------------------------------------------------
