/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: internationaloptions.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:23:46 $
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

#ifndef _OFFAPP_INTERNATIONALOPTIONS_HXX_
#define _OFFAPP_INTERNATIONALOPTIONS_HXX_

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

namespace offapp
{
    class InternationalOptionsPage : public SfxTabPage
    {
    private:
        struct IMPL;
        IMPL*               m_pImpl;

    protected:
                            InternationalOptionsPage( Window* _pParent, const SfxItemSet& _rAttrSet );

        virtual BOOL        FillItemSet( SfxItemSet& _rSet );
        virtual void        Reset( const SfxItemSet& _rSet );

    public:
        virtual             ~InternationalOptionsPage();

        static SfxTabPage*  CreateSd( Window* _pParent, const SfxItemSet& _rAttrSet );
        static SfxTabPage*  CreateSc( Window* _pParent, const SfxItemSet& _rAttrSet );
    };

}   // /namespace offapp

#endif

