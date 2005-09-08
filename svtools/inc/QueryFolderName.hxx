/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: QueryFolderName.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 09:02:15 $
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
#ifndef SVT_QUERYFOLDER_HXX
#define SVT_QUERYFOLDER_HXX
//-------------------------------------------------------------------------
namespace svtools {

//-------------------------------------------------------------------------
// QueryFolderNameDialog
//-------------------------------------------------------------------------

class QueryFolderNameDialog : public ModalDialog
{
private:
    FixedText       aNameText;
    Edit            aNameEdit;
    FixedLine       aNameLine;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;

    DECL_LINK( OKHdl, Button * );
    DECL_LINK( NameHdl, Edit * );

public:
                    QueryFolderNameDialog(  Window* _pParent,
                                            const String& rTitle,
                                            const String& rDefaultText,
                                            String* pGroupName = NULL );
    String          GetName() const { return aNameEdit.GetText(); }
};

}
#endif // SVT_QUERYFOLDER_HXX

