/*************************************************************************
 *
 *  $RCSfile: ThreadId.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: jbu $ $Date: 2002-06-25 07:16:52 $
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
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.environments.remote;


import java.io.UnsupportedEncodingException;


import com.sun.star.uno.UnoRuntime;



/**
 * This is the global thread id.
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2002-06-25 07:16:52 $
 * @author      Joerg Budischewski
 * @see         com.sun.star.lib.uno.environments.remote.ThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.IThreadPool
 * @see         com.sun.star.lib.uno.environments.remote.Job
 */
public class ThreadId {
    static protected byte __async_count;

    protected byte   _threadId[];
    protected int    _hashCode;
    protected String _string;


    /**
     * Constructs a new thread id
     * <p>
     */
    public ThreadId() {
        init(UnoRuntime.generateOid(new Object()));
    }

    /** Use this ctor only as long as the string
        contains only ascii characters. Otherwise,
        use the byte [] ctor.
     */
    public ThreadId(String threadId ) {
        init( threadId );
    }

    /**
     * Constructs a new thread id from the given byte array
     * <p>
     * @param  threadID     a byte array describing a thread id
     */
    public ThreadId(byte threadId[]) {
        init(threadId);
    }

    /**
     * Initializes a thread id with a byte array
     * <p>
     * @param  threadID     a byte array describing a thread id
     */
    private void init(String threadId)
    {
        try {
            _string = threadId;
            _threadId = _string.getBytes( "UTF8" );
        }
        catch(UnsupportedEncodingException unsupportedEncodingException) {
            throw new com.sun.star.uno.RuntimeException(getClass().getName() + ".<init> - unexpected: " + unsupportedEncodingException.toString());
        }
    }

    /**
     * Initializes a thread id with a byte array
     * <p>
     * @param  threadID     a byte array describing a thread id
     */
    private void init(byte threadId[])
    {
        _threadId = threadId;

        // in case the deprecated String( byte [] , byte ) ctor
        // once vanishes, replace it with this  code
//         char [] a = new char[threadId.length];
//         int nMax = threadId.length;
//         for( int i = 0 ; i < nMax ; i ++ )
//             a[i] = (char) threadId[i];
//         _string = new String( a );

        // fast but deprecated
        _string = new String(threadId, (byte) 0 );

    }

    /**
     * Gives a hashcode.
     * <p>
     */
    public int hashCode() {
        //              return _hashCode;
          return _string.hashCode();
    }

    /**
     * Gives a thread id described by a byte array
     * <p>
     * @return   a byte array
     */
    public byte[] getBytes() {
        return _threadId;
    }

    /**
     * Indicates whether two thread ids describe the same threadid
     * <p>
     * @return   <code>true</code>, if the thread ids are equal
     * @param   othreadID    the other thread id
     */
    public boolean equals(Object othreadId) {
          return _string.equals(((ThreadId)othreadId)._string);
    }

    /**
     * Gives a descriptive string
     * <p>
     * @return   the descriptive string
     */
    public String toString() {
        String result = "id:";

        for(int i = 0; i < _threadId.length; ++ i) {
            String tmp = Integer.toHexString(_threadId[i]);
            if(tmp.length() < 2)
                result += "0" + tmp;
            else
                result += tmp.substring(tmp.length() - 2);
        }

        return result;
    }
}


