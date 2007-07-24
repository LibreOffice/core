/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dsselect.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-24 12:08:19 $
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

#include "dsntypes.hxx"
#include "odbcconfig.hxx"
#include "commontypes.hxx"

#include <vcl/dialog.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/button.hxx>
#include <vcl/group.hxx>
#include <vcl/fixed.hxx>
#include <rtl/ustring.hxx>

#include <memory>

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
#ifdef HAVE_ODBC_ADMINISTRATION
    PushButton      m_aManageDatasources;
#endif
    PushButton      m_aCreateAdabasDB;
    SfxItemSet*     m_pOutputSet;
#ifdef HAVE_ODBC_ADMINISTRATION
    ::std::auto_ptr< OOdbcManagement >
                    m_pODBCManagement;
#endif

public:
    ODatasourceSelectDialog( Window* _pParent, const StringBag& _rDatasources, DATASOURCE_TYPE _eType,SfxItemSet* _pOutputSet = NULL );
    ~ODatasourceSelectDialog();

    inline String   GetSelected() const { return m_aDatasource.GetSelectEntry();}
    void            Select( const String& _rEntry ) { m_aDatasource.SelectEntry(_rEntry); }

    virtual BOOL    Close();

protected:
    DECL_LINK( ListDblClickHdl, ListBox * );
#ifdef HAVE_ODBC_ADMINISTRATION
    DECL_LINK( ManageClickHdl, PushButton * );
    DECL_LINK( ManageProcessFinished, void* );
#endif
    DECL_LINK( CreateDBClickHdl, PushButton * );
    void fillListBox(const StringBag& _rDatasources);
};

//.........................................................................
}   // namespace dbaui
//.........................................................................

#endif // _DBAUI_DSSELECT_HXX_

