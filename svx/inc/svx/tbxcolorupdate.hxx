/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tbxcolorupdate.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 07:31:26 $
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

#ifndef SVX_TBXCOLORUPDATE_HXX
#define SVX_TBXCOLORUPDATE_HXX

#include <svx/svxdllapi.h>

#include <tools/gen.hxx>
#include <tools/color.hxx>

class ToolBox;
class VirtualDevice;

//........................................................................
namespace svx
{
//........................................................................

#define TBX_UPDATER_MODE_NONE               0x00
#define TBX_UPDATER_MODE_CHAR_COLOR         0x01
#define TBX_UPDATER_MODE_CHAR_BACKGROUND    0x02
#define TBX_UPDATER_MODE_CHAR_COLOR_NEW     0x03

    //====================================================================
    //= ToolboxButtonColorUpdater
    //====================================================================
    /** helper class to update a color in a toolbox button image

        formerly known as SvxTbxButtonColorUpdater_Impl, residing in svx/source/tbxctrls/colorwindow.hxx.
    */
    class SVX_DLLPUBLIC ToolboxButtonColorUpdater
    {
    public:
                    ToolboxButtonColorUpdater( USHORT   nSlotId,
                                                USHORT   nTbxBtnId,
                                                ToolBox* ptrTbx,
                                                USHORT   nMode = 0 );
                    ~ToolboxButtonColorUpdater();

        void        Update( const Color& rColor );

    protected:
        void        DrawChar(VirtualDevice&, const Color&);

    private:
        USHORT      mnDrawMode;
        USHORT      mnBtnId;
        USHORT      mnSlotId;
        ToolBox*    mpTbx;
        Color       maCurColor;
        Rectangle   maUpdRect;
        Size        maBmpSize;
        BOOL        mbWasHiContrastMode;
    };

//........................................................................
} // namespace svx
//........................................................................

#endif // SVX_TBXCOLORUPDATE_HXX
