/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ViewShellHint.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:19:53 $
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

#ifndef SD_VIEW_SHELL_HINT_HXX
#define SD_VIEW_SHELL_HINT_HXX

#ifndef _SFXHINT_HXX //autogen
#include <svtools/hint.hxx>
#endif

namespace sd {

/** Local derivation of the SfxHint class that defines some hint ids that
    are used by the ViewShell class and its decendants.
*/
class ViewShellHint
    : public SfxHint
{
public:
    enum HintId {
        // Indicate that a page resize is about to begin.
        HINT_PAGE_RESIZE_START,
        // Indicate that a page resize has been completed.
        HINT_PAGE_RESIZE_END,
        // Indicate that an edit mode change is about to begin.
        HINT_CHANGE_EDIT_MODE_START,
        // Indicate that an edit mode change has been completed.
        HINT_CHANGE_EDIT_MODE_END,

        HINT_COMPLEX_MODEL_CHANGE_START,
        HINT_COMPLEX_MODEL_CHANGE_END
    };

    TYPEINFO();

    ViewShellHint (HintId nHintId);

    HintId GetHintId (void) const;

private:
    HintId meHintId;
};

} // end of namespace sd

#endif
