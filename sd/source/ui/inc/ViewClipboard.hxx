/*************************************************************************
 *
 *  $RCSfile: ViewClipboard.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-13 14:03:45 $
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

#ifndef SD_VIEW_CLIPBOARD_HXX
#define SD_VIEW_CLIPBOARD_HXX

#include <tools/solar.h>

class SdPage;
class SdTransferable;

namespace sd {

class View;


/** Handle clipboard related tasks for the draw view.
*/
class ViewClipboard
{
public:
    ViewClipboard (::sd::View& rView);
    virtual ~ViewClipboard (void);

    /** Handle the drop of a drag-and-drop action where the transferable
        contains a set of pages.
    */
    virtual void HandlePageDrop (const SdTransferable& rTransferable);

protected:
    ::sd::View& mrView;

    /** Return the first master page of the given transferable.  When the
        bookmark list of the transferable contains at least one non-master
        page then NULL is returned.
    */
    SdPage* GetFirstMasterPage (const SdTransferable& rTransferable);

    /** Assign the (first) master page of the given transferable to the
        (...) slide.
    */
    virtual void AssignMasterPage (
        const SdTransferable& rTransferable,
        SdPage* pMasterPage);

    /** Return an index of a page after which the pages of the transferable
        are to be inserted into the target document.
    */
    virtual USHORT DetermineInsertPosition (
        const SdTransferable& rTransferable);

    /** Insert the slides in the given transferable behind the last selected
        slide or, when the selection is empty, behind the last slide.
        @param rTransferable
            This transferable defines which pages to insert.
        @param nInsertPosition
            The pages of the transferable will be inserted behind the page
            with this index.
        @return
            Returns the number of inserted slides.
    */
    virtual USHORT InsertSlides (
        const SdTransferable& rTransferable,
        USHORT nInsertPosition);
};

} // end of namespace ::sd

#endif
