/*************************************************************************
 *
 *  $RCSfile: objdlg.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: mh $ $Date: 2000-09-29 11:02:37 $
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

#ifndef _OBJDLG_HXX
#define _OBJDLG_HXX

#ifndef _SVHEADER_HXX
#include <svheader.hxx>
#endif

#ifndef _SV_FLOATWIN_HXX //autogen
#include <vcl/floatwin.hxx>
#endif

#ifndef _SV_TOOLBOX_HXX //autogen
#include <vcl/toolbox.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif

#include <bastype2.hxx>

class StarBASIC;

class ObjectTreeListBox : public BasicTreeListBox
{
private:

    virtual void    Command( const CommandEvent& rCEvt );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );

public:
            ObjectTreeListBox( Window* pParent, const ResId& rRes );
            ~ObjectTreeListBox();
};

class ObjectCatalog : public FloatingWindow
{
private:
    ObjectTreeListBox   aMacroTreeList;
    ToolBox             aToolBox;
    FixedText           aMacroDescr;
    Link                aCancelHdl;

protected:
    DECL_LINK( ToolBoxHdl, ToolBox* );
    void                CheckButtons();
    DECL_LINK( TreeListHighlightHdl, SvTreeListBox * );
    void                UpdateFields();
    virtual void        Move();
    virtual BOOL        Close();
    virtual void        Resize();

public:
    ObjectCatalog( Window * pParent );

    void                ScanBasic( BasicManager* pBasMgr, const String& rName );
    void                UpdateEntries();

//  BYTE                GetSelectedType();
//  BasicManager*       GetSelectedSbx( String& rLib, String& rModOrObj, String& rSubOrProp );

    void                SetCancelHdl( const Link& rLink ) { aCancelHdl = rLink; }
//  void                SetInsertHdl( const Link& rLink ) { aInsertHdl = rLink; }
};

#endif  //_OBJDLG_HXX

