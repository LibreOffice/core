/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PaneShells.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:05:28 $
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

#ifndef SD_PANE_SHELLS_HXX
#define SD_PANE_SHELLS_HXX


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


/** Shell that displays the left pane for Impress.  The shell does not do
    anything else and has especially no slots.
*/
class LeftImpressPaneShell
    : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDLEFTIMPRESSPANESHELL)

    LeftImpressPaneShell (void);
    virtual ~LeftImpressPaneShell (void);
};




/** Shell that displays the left pane for Draw.  The shell does not do
    anything else and has especially no slots.
*/
class LeftDrawPaneShell
    : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDLEFTDRAWPANESHELL)

    LeftDrawPaneShell (void);
    virtual ~LeftDrawPaneShell (void);
};




/** Shell that displays the right pane for both Impress and Draw.  The shell
    does not do anything else and has especially no slots.
*/
class RightPaneShell
    : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE(SD_IF_SDRIGHTPANESHELL)

    RightPaneShell (void);
    virtual ~RightPaneShell (void);
};

} // end of namespace sd

#endif
