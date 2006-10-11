/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: IInterface.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-11 08:45:52 $
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

 #ifndef IINTERFACE_HXX_INCLUDED
 #define IINTERFACE_HXX_INCLUDED

 #include <sal/types.h>

 /** The base interface
 */
 class IInterface
 {
 public:

    /** Aquire a reference to an instance. A caller shall release
        the instance by calling 'release' when it is no longer needed.
        'acquire' and 'release' calls need to be balanced.

        @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 acquire() = 0;

    /** Releases a reference to an instance. A caller has to call
        'release' when a before aquired reference to an instance
        is no longer needed. 'acquire' and 'release' calls need to
        be balanced.

    @returns
        the current reference count of the instance for debugging purposes.
    */
    virtual sal_Int32 release() = 0;

    /** Returns the current reference count. This method should be used for
        debugging purposes. Using it otherwise is a signal of a design flaw.
    */
    virtual sal_Int32 getReferenceCount() const = 0;

protected:
    virtual ~IInterface() {};
 };

 #endif // IDOCUMENT_HXX_INCLUDED
