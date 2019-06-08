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
import com.sun.star.container.*;
import com.sun.star.lang.*;

/**************************************************************************/
/** an abstract interface for components doing an action on a form component
*/
interface IFormComponentAction
{
    public abstract void handle( Object aFormComponent ) throws java.lang.Exception;
};

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
    protected boolean shouldStepInto( XIndexContainer xContainer ) throws com.sun.star.uno.Exception
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

