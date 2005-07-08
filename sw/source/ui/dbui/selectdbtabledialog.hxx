/*************************************************************************
 *
 *  $RCSfile: selectdbtabledialog.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2005-07-08 10:30:46 $
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
#ifndef _SELECTDBTABLEDIALOG_HXX
#define _SELECTDBTABLEDIALOG_HXX

#ifndef _BASEDLGS_HXX
#include <sfx2/basedlgs.hxx>
#endif

#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _HEADBAR_HXX
#include <svtools/headbar.hxx>
#endif
namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XConnection;
    }
}}}

/*-- 08.04.2004 14:04:39---------------------------------------------------

  -----------------------------------------------------------------------*/
class SwSelectDBTableDialog : public SfxModalDialog
{
    FixedText       m_aSelectFI;
    HeaderBar       m_aTableHB;
    SvTabListBox    m_aTableLB;
    PushButton      m_aPreviewPB;

    FixedLine       m_aSeparatorFL;

    OKButton        m_aOK;
    CancelButton    m_aCancel;
    HelpButton      m_aHelp;

    String          m_sName;
    String          m_sType;
    String          m_sTable;
    String          m_sQuery;

    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> m_xConnection;

    DECL_LINK(PreviewHdl, PushButton*);
public:

    SwSelectDBTableDialog(Window* pParent,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& xConnection
            );
    ~SwSelectDBTableDialog();

    String      GetSelectedTable(bool& bIsTable);
    void        SetSelectedTable(const String& rTable, bool bIsTable);
};
#endif

