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



import com.sun.star.uno.*;
import com.sun.star.task.*;
import java.util.Vector;

/**************************************************************************/
/** helper class for implementing an interaction request.
*/
class InteractionRequest implements XInteractionRequest
{
    private Object  m_aRequest;
    private Vector  m_aContinuations;

    /* ------------------------------------------------------------------ */
    public InteractionRequest( Object aRequest )
    {
        m_aRequest = aRequest;
        m_aContinuations = new Vector();
    }

    /* ------------------------------------------------------------------ */
    public void addContinuation( XInteractionContinuation xCont )
    {
        if ( null != xCont )
            m_aContinuations.add( xCont );
    }

    /* ------------------------------------------------------------------ */
    public Object getRequest(  )
    {
        return m_aRequest;
    }

    /* ------------------------------------------------------------------ */
    public XInteractionContinuation[] getContinuations(  )
    {
        XInteractionContinuation[] aContinuations = new XInteractionContinuation[ m_aContinuations.size() ];
        for ( int i=0; i<m_aContinuations.size(); ++i )
            aContinuations[ i ] = (XInteractionContinuation)m_aContinuations.elementAt( i );
        return aContinuations;
    }
};

