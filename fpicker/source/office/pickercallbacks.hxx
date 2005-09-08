/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pickercallbacks.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 23:34:00 $
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

#ifndef SVTOOLS_PICKER_CALLBACKS_HXX
#define SVTOOLS_PICKER_CALLBACKS_HXX

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif
#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class Control;

//.........................................................................
namespace svt
{
//.........................................................................

    // --------------------------------------------------------------------
    class IFilePickerController
    {
    public:
        virtual Control*    getControl( sal_Int16 _nControlId, sal_Bool _bLabelControl = sal_False ) const = 0;
        virtual void        enableControl( sal_Int16 _nControlId, sal_Bool _bEnable ) = 0;
        virtual String      getCurFilter( ) const = 0;
    };

    // --------------------------------------------------------------------
    class IFilePickerListener
    {
    public:
        virtual void    notify( sal_Int16 _nEventId, sal_Int16 _nControlId ) = 0;
    };

//.........................................................................
}   // namespace svt
//.........................................................................

#endif // SVTOOLS_PICKER_CALLBACKS_HXX

