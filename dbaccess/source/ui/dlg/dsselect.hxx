/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsselect.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:03:18 $
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

#ifndef _DBAUI_DSSELECT_HXX_
#define _DBAUI_DSSELECT_HXX_

#ifndef _DBAUI_DSNTYPES_HXX_
#include "dsntypes.hxx"
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

class SfxItemSet;
//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= ODatasourceSelector
//=========================================================================
class ODatasourceSelectDialog : public ModalDialog
{
protected:
    FixedText       m_aDescription;
    ListBox         m_aDatasource;
    OKButton        m_aOk;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;
    PushButton      m_aManageDatasources;
    PushButton      m_aCreateAdabasDB;
    SfxItemSet*     m_pOutputSet;

public:
    ODatasourceSelectDialog(Window* _pParent, const StringBag& _rDatasources, DATASOURCE_TYPE _eType,SfxItemSet* _pOutputSet = NULL);

    inline String   GetSelected() const { return m_aDatasource.GetSelectEntry();}
    void            Select( const String& _rEntry ) { m_aDatasource.SelectEntry(_rEntry); }

protected:
    DECL_LINK( ListDblClickHdl, ListBox * );
    DECL_LINK( ManageClickHdl, PushButton * );
    DECL_LINK( CreateDBClickHdl, PushButton * );
    void fillListBox(const StringBag& _rDatasources);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSSELECT_HXX_

