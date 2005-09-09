/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AllMasterPagesSelector.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:37:48 $
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

#ifndef SD_TOOLPANEL_CONTROLS_ALL_MASTER_PAGES_SELECTOR_HXX
#define SD_TOOLPANEL_CONTROLS_ALL_MASTER_PAGES_SELECTOR_HXX

#include "MasterPagesContainerSelector.hxx"

namespace sd { namespace toolpanel { namespace controls {


/** Show a list of all available master pages and let the user assign them
    to the document.
*/
class AllMasterPagesSelector
    : public MasterPagesContainerSelector
{
public:
    AllMasterPagesSelector (
        TreeNode* pParent,
        SdDrawDocument& rDocument,
        ViewShellBase& rBase,
        DrawViewShell& rViewShell);
    virtual ~AllMasterPagesSelector (void);

    /** Scan the set of templates for the ones whose first master pages are
        shown by this control and store them in the MasterPageContainer.
    */
    virtual void Fill (void);

private:
    DrawViewShell& mrViewShell;
    unsigned int mnCurrentIndex;
    ::std::vector<TemplateEntry*> maTemplates;

    void AddTemplate (const TemplateEntry& rEntry);

    /** This filter returns <TRUE/> when the master page specified by the
        given file name belongs to the set of Impress master pages.
    */
    bool FileFilter (const String& sFileName);
};

} } } // end of namespace ::sd::toolpanel::controls

#endif
