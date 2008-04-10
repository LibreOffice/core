/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: IInterface.hxx,v $
 * $Revision: 1.4 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
