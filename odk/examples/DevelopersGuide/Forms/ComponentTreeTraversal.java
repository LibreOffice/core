/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

import com.sun.star.container.*;
import com.sun.star.lang.*;

/**************************************************************************/
/** an abstract interface for components doing an action on a form component
*/
interface IFormComponentAction
{
    public abstract void handle( Object aFormComponent ) throws java.lang.Exception;
}

/**************************************************************************/
/** a helper class for travelling a form component tree
*/
class ComponentTreeTraversal implements IFormComponentAction
{
    /* ------------------------------------------------------------------ */
    /** Indicator method to decide whether to step down the tree.

        <p>The default implementation checks if the container given is a grid
        control model or a <service scope="com.sun.star.form">FormComponents</service>
        instance.</p>
    */
    protected boolean shouldStepInto( XIndexContainer xContainer )
    {
        // step down the tree, if possible
        XServiceInfo xSI = UNO.queryServiceInfo( xContainer );
        if  (   null != xSI
            &&  (   xSI.supportsService( "com.sun.star.form.FormComponents" )
                ||  xSI.supportsService( "com.sun.star.form.component.GridControl" )
                )
            )
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    /* ------------------------------------------------------------------ */
    public void handle( Object aFormComponent ) throws com.sun.star.uno.Exception
    {
        XIndexContainer xCont = UNO.queryIndexContainer( aFormComponent );
        if  (   ( null != xCont )
            &&  shouldStepInto( xCont )
            )
        {
            for ( int i=0; i<xCont.getCount(); ++i )
            {
                handle( xCont.getByIndex( i ) );
            }
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
