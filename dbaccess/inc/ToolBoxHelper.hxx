/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ToolBoxHelper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:48:48 $
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

#ifndef DBAUI_TOOLBOXHELPER_HXX
#define DBAUI_TOOLBOXHELPER_HXX

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
#ifndef _LINK_HXX
#include <tools/link.hxx>
#endif
#ifndef _SV_GEN_HXX
#include <tools/gen.hxx>
#endif
#ifndef _SV_IMAGE_HXX
#include <vcl/image.hxx>
#endif
#include "dbaccessdllapi.h"

class SvtMiscOptions;
class ToolBox;
class VclWindowEvent;

namespace dbaui
{
    class DBACCESS_DLLPUBLIC OToolBoxHelper
    {
        sal_Bool        m_bIsHiContrast;// true when the toolbox is in hi contrast mode
        sal_Int16       m_nSymbolsSize; // shows the toolbox large or small bitmaps
        ToolBox*        m_pToolBox;     // our toolbox (may be NULL)
    public:
        OToolBoxHelper();
        virtual ~OToolBoxHelper();

        /** will be called when the controls need to be resized.
            @param  _rDiff
                Contains the difference of the old and new toolbox size.
        */
        virtual void resizeControls(const Size& _rDiff) = 0;

        /** will be called when the image list is needed.
            @param  _eSymbolsSize
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eSymbolsSize,sal_Bool _bHiContast) const = 0;

        /** only the member will be set, derived classes can overload this function and do what need to be done.
            @param  _pTB
                The new ToolBox.
            @attention
                Must be called after a FreeResource() call.
        */
        virtual void    setToolBox(ToolBox* _pTB);

        inline ToolBox* getToolBox() const          { return m_pToolBox; }

        /** checks if the toolbox needs a new imagelist.
        */
        void checkImageList();

        inline sal_Bool isToolBoxHiContrast() const { return m_bIsHiContrast; }
    protected:
        DECL_LINK(ConfigOptionsChanged, SvtMiscOptions*);
        DECL_LINK(SettingsChanged, VclWindowEvent* );
    };
}
#endif // DBAUI_TOOLBOXHELPER_HXX

