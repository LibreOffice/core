/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: vclwrapper.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 23:05:54 $
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

#ifndef _CANVAS_VCLWRAPPER_HXX
#define _CANVAS_VCLWRAPPER_HXX

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif


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
                ::vos::OGuard aGuard( Application::GetSolarMutex() );

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

#endif /* _CANVAS_VCLWRAPPER_HXX */
