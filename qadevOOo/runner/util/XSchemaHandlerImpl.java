/*************************************************************************
 *
 *  $RCSfile: XSchemaHandlerImpl.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:18:34 $
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
package util;

public class XSchemaHandlerImpl
    implements com.sun.star.configuration.backend.XSchemaHandler {
    protected String calls = "";
    protected String ls = System.getProperty("line.separator");

    public void addInstance(String str,
                            com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addInstance();" + ls);
    }

    public void addItemType(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addItemType();" + ls);
    }

    public void addProperty(String str, short param,
                            com.sun.star.uno.Type type)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addProperty();" + ls);
    }

    public void addPropertyWithDefault(String str, short param, Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addPropertyWithDefault();" + ls);
    }

    public void endComponent()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endComponent();" + ls);
    }

    public void endNode()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endNode();" + ls);
    }

    public void endSchema()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endSchema();" + ls);
    }

    public void endTemplate()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endTemplate();" + ls);
    }

    public void importComponent(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("importComponent();" + ls);
    }

    public void startComponent(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startComponent();" + ls);
    }

    public void startGroup(String str, short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startGroup();" + ls);
    }

    public void startGroupTemplate(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                   short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startGroupTemplate();" + ls);
    }

    public void startSchema()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSchema();" + ls);
    }

    public void startSet(String str, short param,
                         com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSet();" + ls);
    }

    public void startSetTemplate(com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                 short param,
                                 com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier2)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("startSetTemplate();" + ls);
    }

    public String getCalls() {
        return calls;
    }

    public void cleanCalls() {
        calls = "";
    }
}