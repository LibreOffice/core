/*************************************************************************
 *
 *  $RCSfile: pyuno_gc.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jbu $ $Date: 2003-03-23 12:12:57 $
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
 *  The Initial Developer of the Original Code is: Ralph Thomas
 *
 *   Copyright: 2000 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *   Contributor(s): Ralph Thomas, Joerg Budischewski
 *
 *
 ************************************************************************/
#include <osl/thread.hxx>
#include <pyuno_impl.hxx>
namespace pyuno
{


class GCThread : public ::osl::Thread
{
    PyObject *mPyObject;
    PyInterpreterState *mPyInterpreter;
    GCThread( const GCThread & ); // not implemented
    GCThread &operator =( const GCThread & ); // not implemented

public:
    GCThread( PyInterpreterState *interpreter, PyObject * object );
    virtual void SAL_CALL run();
    virtual void SAL_CALL onTerminated();
};


GCThread::GCThread( PyInterpreterState *interpreter, PyObject * object ) :
    mPyInterpreter( interpreter ), mPyObject( object )
{}

void GCThread::run()
{
    try
    {
        PyThreadAttach guard( (PyInterpreterState*)mPyInterpreter );
        {
            Py_XDECREF( mPyObject );
        }
    }
    catch( com::sun::star::uno::RuntimeException & e )
    {
        rtl::OString msg;
        msg = rtl::OUStringToOString( e.Message, RTL_TEXTENCODING_ASCII_US );
        fprintf( stderr, "Leaking python objects bridged to UNO for reason %s\n",msg.getStr());
    }
}


void GCThread::onTerminated()
{
    delete this;
}

void decreaseRefCount( PyInterpreterState *interpreter, PyObject *object )
{
    // delegate to a new thread, because there does not seem
    // to be a method, which tells, whether the global
    // interpreter lock is held or not
    // TODO: Look for a more efficient solution
    osl::Thread *t = new GCThread( interpreter, object );
    t->create();
}

}
