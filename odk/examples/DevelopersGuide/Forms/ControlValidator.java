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



/** base class for components validating the content of form controls
 *
 * @author  fs@openoffice.org
 */
public abstract class ControlValidator implements com.sun.star.form.validation.XValidator
{

    /** Creates a new instance of ControlValidator */
    public ControlValidator()
    {
    }

    public void addValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
    {
    }

    public void removeValidityConstraintListener(com.sun.star.form.validation.XValidityConstraintListener xValidityConstraintListener)
    {
    }

    protected boolean isVoid( Object Value )
    {
        try
        {
            return ( com.sun.star.uno.AnyConverter.getType(Value).getTypeClass()
                     == com.sun.star.uno.TypeClass.VOID );
        }
        catch( java.lang.ClassCastException e )
        {
        }
        return false;
    }
}
