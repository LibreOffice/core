/*************************************************************************
 *
 *  $RCSfile: TitleHelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-06 09:58:29 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART2_TOOLS_TITLEHELPER_HXX
#define _CHART2_TOOLS_TITLEHELPER_HXX

#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
#endif

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif

//.............................................................................
namespace chart
{
//.............................................................................

class TitleHelper
{
public:
    enum eTitleType
    {
        MAIN_TITLE,
        SUB_TITLE,
        X_AXIS_TITLE,
        Y_AXIS_TITLE,
        Z_AXIS_TITLE,
        SECOND_X_AXIS_TITLE,
        SECOND_Y_AXIS_TITLE
    };

    static rtl::OUString getIdentifierForTitle( eTitleType nTitleIndex );

    static ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XTitle >
        getTitle( eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                    ::com::sun::star::frame::XModel >& xModel );

    static ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XTitle >
        createTitle(  eTitleType nTitleIndex
                    , const rtl::OUString& rTitleText
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::uno::XComponentContext > & xContext );

    static void removeTitle( eTitleType nTitleIndex
                    , const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XModel >& xModel );

    static rtl::OUString getCompleteString( const ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XTitle >& xTitle );
    static void setCompleteString( const rtl::OUString& rNewText
        , const ::com::sun::star::uno::Reference<
            ::drafts::com::sun::star::chart2::XTitle >& xTitle
        , const ::com::sun::star::uno::Reference<
            ::com::sun::star::uno::XComponentContext > & xContext );
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
