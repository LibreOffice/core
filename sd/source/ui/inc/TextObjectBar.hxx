/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TextObjectBar.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:39:12 $
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

#ifndef SD_TEXT_OBJECT_BAR_HXX
#define SD_TEXT_OBJECT_BAR_HXX


#ifndef _SFXMODULE_HXX //autogen
#include <sfx2/module.hxx>
#endif
#ifndef _SFX_SHELL_HXX //autogen
#include <sfx2/shell.hxx>
#endif

#ifndef _SD_GLOB_HXX
#include "glob.hxx"
#endif

class CommandEvent;

namespace sd {

class View;
class ViewShell;
class Window;

class TextObjectBar
    : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDDRAWTEXTOBJECTBAR)

    TextObjectBar (
        ViewShell* pSdViewShell,
        SfxItemPool& rItemPool,
        ::sd::View* pSdView);
    virtual ~TextObjectBar (void);

    void GetAttrState( SfxItemSet& rSet );
    void Execute( SfxRequest &rReq );

    virtual void Command( const CommandEvent& rCEvt );

private:
    ViewShell* mpViewShell;
    ::sd::View* mpView;
};

} // end of namespace sd

#endif
