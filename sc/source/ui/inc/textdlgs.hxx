/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: textdlgs.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:58:08 $
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

#ifndef SC_TEXTDLGS_HXX
#define SC_TEXTDLGS_HXX

#ifndef _SFXTABDLG_HXX //autogen
#include <sfx2/tabdlg.hxx>
#endif

class SfxObjectShell;

//
//      Zeichen-Tab-Dialog
//

class ScCharDlg : public SfxTabDialog
{
private:
    const SfxItemSet&       rOutAttrs;
    const SfxObjectShell&   rDocShell;

    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );

public:
            ScCharDlg( Window* pParent, const SfxItemSet* pAttr,
                        const SfxObjectShell* pDocShell );
            ~ScCharDlg() {}
};

//
//      Absatz-Tab-Dialog
//

class ScParagraphDlg : public SfxTabDialog
{
private:
    const SfxItemSet&   rOutAttrs;

    virtual void    PageCreated( USHORT nId, SfxTabPage &rPage );

public:
            ScParagraphDlg( Window* pParent, const SfxItemSet* pAttr );
            ~ScParagraphDlg() {}
};



#endif

