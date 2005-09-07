/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: JNI_info_holder.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:38:49 $
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

package com.sun.star.bridges.jni_uno;

import com.sun.star.lib.util.NativeLibraryLoader;

//==============================================================================
public final class JNI_info_holder
{
    static {
        NativeLibraryLoader.loadLibrary(JNI_info_holder.class.getClassLoader(),
                                        "java_uno");
    }

    private static JNI_info_holder s_holder = new JNI_info_holder();

    private static long s_jni_info_handle;

    //__________________________________________________________________________
    private native void finalize( long jni_info_handle );

    //__________________________________________________________________________
    protected void finalize()
    {
        finalize( s_jni_info_handle );
    }
}
