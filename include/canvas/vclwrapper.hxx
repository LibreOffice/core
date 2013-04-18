/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_CANVAS_VCLWRAPPER_HXX
#define INCLUDED_CANVAS_VCLWRAPPER_HXX

#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>

namespace canvas
{
    namespace vcltools
    {
        /** This helper template wraps VCL objects, and protects
            object deletion with the Solar mutex. All other operations
            are unprotected, this must be handled by client code.

            The reason for this template is the fact that VCL objects
            hold by value in uno::Reference-handled classes are
            deleted without having the chance to get inbetween and
            lock the solar mutex.

            This template handles that problem transparently, the only
            inconvenience is the fact that object member access now
            has to be performed via operator->, since . is not
            overloadable.

            Otherwise, the template preserves the value semantics of
            the wrapped object, that is, copy operations are performed
            not by copying pointers, but by copying the underlying
            object. This includes constness, i.e. on a const
            VCLObject, only const methods of the wrapped object can be
            called. Simply imagine this to be a value object of type
            "template argument", with the only peculiarity that
            member/method access is performed by operator-> instead of
            the non-existing "operator.".
         */
        template< class _Wrappee > class VCLObject
        {
        public:
            typedef _Wrappee Wrappee;

            VCLObject() :
                mpWrappee( new Wrappee() )
            {
            }

            // no explicit here. VCLObjects should be freely
            // constructible with Wrappees, and AFAIK there is no other
            // implicit conversion path that could cause harm here
            VCLObject( Wrappee* pWrappee ) :
                mpWrappee( pWrappee )
            {
            }

            // This object has value semantics, thus, forward copy
            // to wrappee
            VCLObject( const VCLObject& rOrig )
            {
                if( rOrig.mpWrappee )
                    mpWrappee = new Wrappee( *rOrig.mpWrappee );
                else
                    mpWrappee = NULL;
            }

            // This object has value semantics, thus, forward copy
            // to wrappee
            VCLObject( const Wrappee& rOrig ) :
                mpWrappee( new Wrappee( rOrig ) )
            {
            }

            // This object has value semantics, thus, forward
            // assignment to wrappee
            VCLObject& operator=( const VCLObject& rhs )
            {
                if( mpWrappee )
                {
                    if( rhs.mpWrappee )
                        *mpWrappee = *rhs.mpWrappee;
                }
                else
                {
                    if( rhs.mpWrappee )
                        mpWrappee = new Wrappee( *rhs.mpWrappee );
                }

                return *this;
            }

            ~VCLObject()
            {
                // This here is the whole purpose of the template:
                // protecting object deletion with the solar mutex
                SolarMutexGuard aGuard;

                if( mpWrappee )
                    delete mpWrappee;
            }

            Wrappee*        operator->() { return mpWrappee; }
            const Wrappee*  operator->() const { return mpWrappee; }

            Wrappee&        operator*() { return *mpWrappee; }
            const Wrappee&  operator*() const { return *mpWrappee; }

            Wrappee&        get() { return *mpWrappee; }
            const Wrappee&  get() const { return *mpWrappee; }

            void swap( VCLObject& rOther )
            {
                ::std::swap( mpWrappee, rOther.mpWrappee );
            }

        private:

            Wrappee* mpWrappee;
        };

    }
}

#endif /* INCLUDED_CANVAS_VCLWRAPPER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
