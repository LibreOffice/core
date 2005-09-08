/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: queryview.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 16:01:50 $
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
#ifndef DBAUI_QUERYVIEW_HXX
#define DBAUI_QUERYVIEW_HXX

#ifndef DBAUI_JOINDESIGNVIEW_HXX
#include "JoinDesignView.hxx"
#endif

namespace dbaui
{
    class OQueryController;
    class OQueryView : public OJoinDesignView
    {
    public:
        OQueryView(Window* pParent, OQueryController* _pController,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& );
        virtual ~OQueryView();

        virtual sal_Bool isCutAllowed()     = 0;
        virtual sal_Bool isPasteAllowed()   = 0;
        virtual sal_Bool isCopyAllowed()    = 0;
        virtual void copy()     = 0;
        virtual void cut()      = 0;
        virtual void paste()    = 0;
        // clears the whole query
        virtual void clear() = 0;
        // set the view readonly or not
        virtual void setReadOnly(sal_Bool _bReadOnly) = 0;
        // set the statement for representation
        virtual void setStatement(const ::rtl::OUString& _rsStatement) = 0;
        // returns the current sql statement
        virtual ::rtl::OUString getStatement() = 0;
    };
}
#endif // DBAUI_QUERYVIEW_HXX
