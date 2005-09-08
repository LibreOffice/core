/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgattr.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:52:52 $
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
#ifndef DBAUI_SBATTRDLG_HXX
#define DBAUI_SBATTRDLG_HXX


#ifndef DBAUI_SBATTRDLG_HRC
#include "dlgattr.hrc"
#endif

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif



class SvxNumberInfoItem;
class SfxItemSet;
class SvNumberFormatter;
//.........................................................................
namespace dbaui
{
//.........................................................................

    class SbaSbAttrDlg : public SfxTabDialog
    {
        String aTitle;
        SvxNumberInfoItem* pNumberInfoItem;

    public:
        SbaSbAttrDlg( Window * pParent, const SfxItemSet*, SvNumberFormatter*, USHORT nFlags = TP_ATTR_CHAR , BOOL bRow = FALSE);
        ~SbaSbAttrDlg();

        virtual void  PageCreated( USHORT nPageId, SfxTabPage& rTabPage );
    };
//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif//DBAUI_SBATTRDLG_HXX

