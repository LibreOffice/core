/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


package integration.forms;

class WaitForInput extends java.lang.Thread
{
    private Object      m_aToNotify;
    private boolean     m_bDone;

    public WaitForInput( Object aToNotify )
    {
        m_aToNotify = aToNotify;
        m_bDone = false;
    }

    public boolean isDone()
    {
        return m_bDone;
    }

    public void run()
    {
        try
        {
            System.out.println( "\npress enter to exit" );
            System.in.read();

            m_bDone = true;
            // notify that the user pressed the key
            synchronized (m_aToNotify)
            {
                m_aToNotify.notify();
            }
        }
        catch( java.lang.Exception e )
        {
            // not really interested in
            System.err.println( e );
        }
    }
};

