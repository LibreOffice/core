/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: WExtendPages.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-30 08:48:48 $
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
#ifndef DBAUI_WIZ_EXTENDPAGES_HXX
#define DBAUI_WIZ_EXTENDPAGES_HXX

#ifndef DBAUI_WIZ_TYPESELECT_HXX
#include "WTypeSelect.hxx"
#endif

class SvStream;
namespace dbaui
{
    // ========================================================
    // Wizard Page: OWizHTMLExtend
    // ========================================================
    class OWizHTMLExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser*   createReader(sal_Int32 _nRows);
    public:
        OWizHTMLExtend(Window* pParent, SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizHTMLExtend( _pParent, _rInput ); }

        virtual ~OWizHTMLExtend(){}
    };
    // ========================================================
    // Wizard Page: OWizRTFExtend
    // ========================================================
    class OWizRTFExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizRTFExtend(Window* pParent,SvStream& _rStream)
            : OWizTypeSelect( pParent, &_rStream )
        {
        }

        static OWizTypeSelect* Create( Window* _pParent, SvStream& _rInput ) { return new OWizRTFExtend( _pParent, _rInput ); }

        virtual ~OWizRTFExtend(){}
    };

    // ========================================================
    // Wizard Page: OWizNormalExtend
    // ========================================================
    class OWizNormalExtend : public OWizTypeSelect
    {
    protected:
        virtual SvParser* createReader(sal_Int32 _nRows);
    public:
        OWizNormalExtend(Window* pParent);
    };
}
#endif // DBAUI_WIZ_EXTENDPAGES_HXX



