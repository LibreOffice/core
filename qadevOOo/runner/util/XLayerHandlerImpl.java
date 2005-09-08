/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: XLayerHandlerImpl.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:39:07 $
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
package util;

public class XLayerHandlerImpl
    implements com.sun.star.configuration.backend.XLayerHandler {
    protected String calls = "";
    protected String ls = System.getProperty("line.separator");

    public void addOrReplaceNode(String str, short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addOrReplace(" + str + "," + param + ");" + ls);
    }

    public void addOrReplaceNodeFromTemplate(String str,
                                             com.sun.star.configuration.backend.TemplateIdentifier templateIdentifier,
                                             short param)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addOrReplaceNodeFromTemplate(" + str + "," + templateIdentifier + ");" + ls);
    }

    public void addProperty(String str, short param,
                            com.sun.star.uno.Type type)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addProperty(" + str + "," + param + "," + type + ");" + ls);
    }

    public void addPropertyWithValue(String str, short param, Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("addPropertyWithValue(" + str + "," + param + "," + obj + ");" + ls);
    }

    public void dropNode(String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("dropNode(" + str + ");" + ls);
    }

    public void endLayer()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endLayer();" + ls);
    }

    public void endNode()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endNode();" + ls);
    }

    public void endProperty()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("endProperty();" + ls);
    }

    public void overrideNode(String str, short param, boolean param2)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("overrideNode(" + str + "," + param + "," + param2 + ");" + ls);
    }

    public void overrideProperty(String str, short param,
                                 com.sun.star.uno.Type type, boolean param3)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("overrideProperty(" + str + "," + param + "," + type + "," + param3 + ");" + ls);
    }

    public void setPropertyValue(Object obj)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("setPropertyValue(" + obj + ");" + ls);
    }

    public void setPropertyValueForLocale(Object obj, String str)
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls += ("setPropertyValueForLocale(" + obj + "," + str + ");" + ls);
    }

    public void startLayer()
        throws com.sun.star.configuration.backend.MalformedDataException,
               com.sun.star.lang.WrappedTargetException {
        calls = "startLayer();" + ls;
    }

    public String getCalls() {
        return calls;
    }
}