/*************************************************************************
 *
 *  $RCSfile: dlgfact.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-01-05 11:32:21 $
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
#ifndef _CUI_DLGFACT_HXX
#define _CUI_DLGFACT_HXX

// include ---------------------------------------------------------------

#include "svxdlg.hxx"

class SfxTabDialog;
class Dialog;

class AbstractDialog : public VclAbstractDialog
{
    Dialog*                     pDlg;
public:
            AbstractDialog( Dialog* p)
                : pDlg(p)
            {}

    virtual                     ~AbstractDialog();
    virtual USHORT              Execute();
    virtual void                Show( BOOL bShow );
    virtual String              GetData();
};

class AbstractTabDialog : public SfxAbstractTabDialog
{
    SfxTabDialog* pDlg;
public:
            AbstractTabDialog( SfxTabDialog* p)
                : pDlg(p)
            {}

    virtual                     ~AbstractTabDialog();
    virtual USHORT              Execute();
    virtual void                Show( BOOL bShow );
    virtual void                SetCurrentPageId( USHORT nId );
    virtual const SfxItemSet*   GetOutputItemSet() const;
    virtual String              GetData();
};

class AbstractDialogFactory : public SvxAbstractDialogFactory
{

public:
    virtual VclAbstractDialog*          CreateVclDialog( Window* pParent, const ResId& rResId );
    virtual VclAbstractDialog*          CreateSfxDialog( Window* pParent, const SfxBindings& rBindings, const ResId& rResId );
    virtual SfxAbstractTabDialog*       CreateTabDialog( const ResId& rResId,
                                            Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            SfxViewFrame* pViewFrame,
                                            bool bEditFmt=false,
                                            const String *pUserButtonText=0 );
    virtual SfxAbstractTabDialog*       CreateDialog( Window* pParent,
                                            const SfxItemSet* pAttrSet,
                                            const ResId& rResId,
                                            SdrView* pView,
                                            SdrModel* pModel=0 );
    virtual CreateTabPage               GetTabPageCreatorFunc( USHORT nId );
    virtual GetTabPageRanges            GetTabPageRangesFunc( USHORT nId );
};

#endif

