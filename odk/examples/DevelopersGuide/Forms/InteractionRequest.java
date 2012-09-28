/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the BSD license.
 *
 *  Copyright 2000, 2010 Oracle and/or its affiliates.
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3. Neither the name of Sun Microsystems, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 *  OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 *  TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 *  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *************************************************************************/

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

