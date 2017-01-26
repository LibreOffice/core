/* -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
import com.sun.star.sheet.DataPilotFieldFilter;

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

//  Example DataPilot source component

//  helper class to hold the settings

class ExampleSettings
{
    public static final int nDimensionCount = 6;
    public static final int nValueDimension = 4;
    public static final int nDataDimension = 5;
    public static final String [] aDimensionNames = {
        "ones", "tens", "hundreds", "thousands", "value", "" };

    public static final String getMemberName( int nMember )
    {
        return String.valueOf( nMember );
    }

    public int nMemberCount = 3;
    public java.util.List<Integer> aColDimensions = new java.util.ArrayList<Integer>();
    public java.util.List<Integer> aRowDimensions = new java.util.ArrayList<Integer>();
}

//  XPropertySetInfo implementation for getPropertySetInfo

class ExamplePropertySetInfo implements com.sun.star.beans.XPropertySetInfo
{
    private final com.sun.star.beans.Property[] aProperties;

    public ExamplePropertySetInfo( com.sun.star.beans.Property[] aProps )
    {
        aProperties = aProps;
    }

    public com.sun.star.beans.Property[] getProperties()
    {
        return aProperties;
    }

    public com.sun.star.beans.Property getPropertyByName( String aName )
                    throws com.sun.star.beans.UnknownPropertyException
    {
        for ( int i=0; i<aProperties.length; i++ )
            if ( aProperties[i].Name.equals( aName ) )
                return aProperties[i];
        throw new com.sun.star.beans.UnknownPropertyException();
    }

    public boolean hasPropertyByName( String aName )
    {
        for ( int i=0; i<aProperties.length; i++ )
            if ( aProperties[i].Name.equals( aName ) )
                return true;
        return false;
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceMember

class ExampleMember implements com.sun.star.container.XNamed,
      com.sun.star.beans.XPropertySet
{
    private final int nMember;

    public ExampleMember( int nMbr )
    {
        nMember = nMbr;
    }

    //  XNamed

    public String getName()
    {
        return ExampleSettings.getMemberName( nMember );
    }

    public void setName( String aName )
    {
        // ignored
    }

    //  XPropertySet

    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return new ExamplePropertySetInfo( new com.sun.star.beans.Property[] {
            new com.sun.star.beans.Property( "IsVisible",   -1,
                        new com.sun.star.uno.Type( Boolean.class ), (short) 0),
            new com.sun.star.beans.Property( "ShowDetails", -1,
                        new com.sun.star.uno.Type( Boolean.class ), (short) 0) });
    }

    public void setPropertyValue( String aPropertyName, Object aValue )
        throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "IsVisible" ) ||
             aPropertyName.equals( "ShowDetails" ) )
        {
            // ignored
        }
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public Object getPropertyValue( String aPropertyName )
        throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "IsVisible" ) ||
             aPropertyName.equals( "ShowDetails" ) )
        {
            return Boolean.TRUE;     // always true
        }
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public void addPropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener)
    {
    }
    public void removePropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener aListener)
    {
    }
    public void addVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
    public void removeVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceMembers

class ExampleMembers implements com.sun.star.sheet.XMembersAccess
{
    private final ExampleSettings aSettings;
    private ExampleMember[] aMembers;

    public ExampleMembers( ExampleSettings aSet )
    {
        aSettings = aSet;
        aMembers = new ExampleMember[ aSettings.nMemberCount ];
    }

    //  XNameAccess

    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type( com.sun.star.container.XNamed.class );
    }

    public boolean hasElements()
    {
        return true;    // always has elements
    }

    public Object getByName( String aName )
        throws com.sun.star.container.NoSuchElementException
    {
        int nCount = aSettings.nMemberCount;
        for ( int i=0; i<nCount; i++ )
            if ( aName.equals( ExampleSettings.getMemberName( i ) ) )
            {
                if ( aMembers[i] == null )
                    aMembers[i] = new ExampleMember( i );
                return aMembers[i];
            }
        throw new com.sun.star.container.NoSuchElementException();
    }

    public String[] getElementNames()
    {
        int nCount = aSettings.nMemberCount;
        String [] aNames = new String[ nCount ];
        for ( int i=0; i<nCount; i++ )
            aNames[i] = ExampleSettings.getMemberName( i );
        return aNames;
    }

    public boolean hasByName( String aName )
    {
        int nCount = aSettings.nMemberCount;
        for ( int i=0; i<nCount; i++ )
            if ( aName.equals( ExampleSettings.getMemberName( i ) ) )
                return true;
        return false;
    }

    // XMembersAccess

    public String[] getLocaleIndependentElementNames()
    {
        return getElementNames();
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceLevel

class ExampleLevel implements
                    com.sun.star.container.XNamed,
                    com.sun.star.sheet.XMembersSupplier,
                    com.sun.star.sheet.XDataPilotMemberResults,
                    com.sun.star.beans.XPropertySet
{
    private final ExampleSettings aSettings;
    private final int nDimension;
    private ExampleMembers aMembers;

    public ExampleLevel( ExampleSettings aSet, int nDim )
    {
        aSettings = aSet;
        nDimension = nDim;
    }

    // XNamed

    public String getName()
    {
        return ExampleSettings.aDimensionNames[ nDimension ];
    }

    public void setName( String aName )
    {
        // ignored
    }

    // XMembersSupplier

    public com.sun.star.sheet.XMembersAccess getMembers()
    {
        if ( aMembers == null )
            aMembers = new ExampleMembers( aSettings );
        return aMembers;
    }

    // XDataPilotMemberResults

    public com.sun.star.sheet.MemberResult[] getResults()
    {
        int nDimensions = 0;
        int nPosition = aSettings.aColDimensions.indexOf( Integer.valueOf(nDimension));
        if ( nPosition >= 0 )
            nDimensions = aSettings.aColDimensions.size();
        else
        {
            nPosition = aSettings.aRowDimensions.indexOf( Integer.valueOf(nDimension));
            if ( nPosition >= 0 )
                nDimensions = aSettings.aRowDimensions.size();
        }

        if ( nPosition < 0 )
            return new com.sun.star.sheet.MemberResult[0];

        int nMembers = aSettings.nMemberCount;
        int nRepeat = 1;
        int nFill = 1;
        for ( int i=0; i<nDimensions; i++ )
        {
            if ( i < nPosition )
                nRepeat *= nMembers;
            else if ( i > nPosition )
                nFill *= nMembers;
        }
        int nSize = nRepeat * nMembers * nFill;

        com.sun.star.sheet.MemberResult[] aResults =
            new com.sun.star.sheet.MemberResult[nSize];
        int nResultPos = 0;
        for (int nOuter=0; nOuter<nRepeat; nOuter++)
        {
            for (int nMember=0; nMember<nMembers; nMember++)
            {
                aResults[nResultPos] = new com.sun.star.sheet.MemberResult();
                aResults[nResultPos].Name = ExampleSettings.getMemberName(nMember);
                aResults[nResultPos].Caption = aResults[nResultPos].Name;
                aResults[nResultPos].Flags =
                    com.sun.star.sheet.MemberResultFlags.HASMEMBER;
                ++nResultPos;

                for (int nInner=1; nInner<nFill; nInner++)
                {
                    aResults[nResultPos] = new com.sun.star.sheet.MemberResult();
                    aResults[nResultPos].Flags =
                        com.sun.star.sheet.MemberResultFlags.CONTINUE;
                    ++nResultPos;
                }
            }
        }
        return aResults;
    }

    //  XPropertySet

    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return new ExamplePropertySetInfo( new com.sun.star.beans.Property[] {
            new com.sun.star.beans.Property( "SubTotals", -1,
                            new com.sun.star.uno.Type(
                                com.sun.star.sheet.GeneralFunction[].class ),
                                             (short) 0 ),
            new com.sun.star.beans.Property( "ShowEmpty", -1,
                             new com.sun.star.uno.Type( Boolean.class ),
                                             (short) 0 ) } );
    }

    public void setPropertyValue( String aPropertyName, Object aValue )
        throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "SubTotals" ) ||
             aPropertyName.equals( "ShowEmpty" ) )
        {
            // ignored
        }
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public Object getPropertyValue( String aPropertyName )
        throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "SubTotals" ) )
            return new com.sun.star.sheet.GeneralFunction[0];
        else if ( aPropertyName.equals( "ShowEmpty" ) )
            return Boolean.TRUE;
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public void addPropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener)
    {
    }
    public void removePropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener aListener)
    {
    }
    public void addVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
    public void removeVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceLevels

class ExampleLevels implements com.sun.star.container.XNameAccess
{
    private final ExampleSettings aSettings;
    private final int nDimension;
    private ExampleLevel aLevel;

    public ExampleLevels( ExampleSettings aSet, int nDim )
    {
        aSettings = aSet;
        nDimension = nDim;
    }

    // XNameAccess

    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type( com.sun.star.container.XNamed.class );
    }

    public boolean hasElements()
    {
        return true;    // always has elements
    }

    public Object getByName( String aName )
        throws com.sun.star.container.NoSuchElementException
    {
        //  there's only one level with the same name as the dimension / hierarchy
        if ( aName.equals( ExampleSettings.aDimensionNames[nDimension] ) )
        {
            if ( aLevel == null )
                aLevel = new ExampleLevel( aSettings, nDimension );
            return aLevel;
        }
        throw new com.sun.star.container.NoSuchElementException();
    }

    public String[] getElementNames()
    {
        String [] aNames = new String[ 1 ];
        aNames[0] = ExampleSettings.aDimensionNames[nDimension];
        return aNames;
    }

    public boolean hasByName( String aName )
    {
        return aName.equals( ExampleSettings.aDimensionNames[nDimension] );
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceHierarchy

class ExampleHierarchy implements com.sun.star.container.XNamed,
      com.sun.star.sheet.XLevelsSupplier
{
    private final ExampleSettings aSettings;
    private final int nDimension;
    private ExampleLevels aLevels;

    public ExampleHierarchy( ExampleSettings aSet, int nDim )
    {
        aSettings = aSet;
        nDimension = nDim;
    }

    // XNamed

    public String getName()
    {
        return ExampleSettings.aDimensionNames[ nDimension ];
    }

    public void setName( String aName )
    {
        // ignored
    }

    // XLevelsSupplier

    public com.sun.star.container.XNameAccess getLevels()
    {
        if ( aLevels == null )
            aLevels = new ExampleLevels( aSettings, nDimension );
        return aLevels;
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceHierarchies

class ExampleHierarchies implements com.sun.star.container.XNameAccess
{
    private final ExampleSettings aSettings;
    private final int nDimension;
    private ExampleHierarchy aHierarchy;

    public ExampleHierarchies( ExampleSettings aSet, int nDim )
    {
        aSettings = aSet;
        nDimension = nDim;
    }

    //  XNameAccess

    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type( com.sun.star.container.XNamed.class );
    }

    public boolean hasElements()
    {
        return true;    // always has elements
    }

    public Object getByName( String aName )
        throws com.sun.star.container.NoSuchElementException
    {
        //  there's only one hierarchy with the same name as the dimension
        if ( aName.equals( ExampleSettings.aDimensionNames[nDimension] ) )
        {
            if ( aHierarchy == null )
                aHierarchy = new ExampleHierarchy( aSettings, nDimension );
            return aHierarchy;
        }
        throw new com.sun.star.container.NoSuchElementException();
    }

    public String[] getElementNames()
    {
        String [] aNames = new String[ 1 ];
        aNames[0] = ExampleSettings.aDimensionNames[nDimension];
        return aNames;
    }

    public boolean hasByName( String aName )
    {
        return aName.equals( ExampleSettings.aDimensionNames[nDimension] );
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceDimension

class ExampleDimension implements
                    com.sun.star.container.XNamed,
                    com.sun.star.sheet.XHierarchiesSupplier,
                    com.sun.star.util.XCloneable,
                    com.sun.star.beans.XPropertySet
{
    private final ExampleSettings aSettings;
    private final int nDimension;
    private ExampleHierarchies aHierarchies;
    private com.sun.star.sheet.DataPilotFieldOrientation eOrientation;

    public ExampleDimension( ExampleSettings aSet, int nDim )
    {
        aSettings = aSet;
        nDimension = nDim;
        eOrientation = ( nDim == ExampleSettings.nValueDimension ) ?
            com.sun.star.sheet.DataPilotFieldOrientation.DATA :
            com.sun.star.sheet.DataPilotFieldOrientation.HIDDEN;
    }

    //  XNamed

    public String getName()
    {
        return ExampleSettings.aDimensionNames[ nDimension ];
    }

    public void setName( String aName )
    {
        // ignored
    }

    //  XHierarchiesSupplier

    public com.sun.star.container.XNameAccess getHierarchies()
    {
        if ( aHierarchies == null )
            aHierarchies = new ExampleHierarchies( aSettings, nDimension );
        return aHierarchies;
    }

    //  XCloneable

    public com.sun.star.util.XCloneable createClone()
    {
        return null;        // not supported
    }

    //  XPropertySet

    public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
    {
        return new ExamplePropertySetInfo( new com.sun.star.beans.Property[] {
            new com.sun.star.beans.Property( "Original", -1,
                new com.sun.star.uno.Type( com.sun.star.container.XNamed.class),
                com.sun.star.beans.PropertyAttribute.READONLY ),
            new com.sun.star.beans.Property( "IsDataLayoutDimension", -1,
                new com.sun.star.uno.Type( Boolean.class),
                com.sun.star.beans.PropertyAttribute.READONLY ),
            new com.sun.star.beans.Property( "Orientation", -1,
                new com.sun.star.uno.Type(
                  com.sun.star.sheet.DataPilotFieldOrientation.class), (short) 0),
            new com.sun.star.beans.Property( "Position", -1,
                new com.sun.star.uno.Type( Integer.class ), (short) 0),
            new com.sun.star.beans.Property( "Function", -1,
                new com.sun.star.uno.Type(com.sun.star.sheet.GeneralFunction.class),
                                             (short) 0 ),
            new com.sun.star.beans.Property( "UsedHierarchy", -1,
                new com.sun.star.uno.Type( Integer.class ), (short) 0 ),
            new com.sun.star.beans.Property( "Filter", -1,
                new com.sun.star.uno.Type(
                    com.sun.star.sheet.TableFilterField[].class), (short) 0) });
    }

    public void setPropertyValue( String aPropertyName, Object aValue )
                    throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "Orientation" ) )
        {
            com.sun.star.sheet.DataPilotFieldOrientation eNewOrient =
                        (com.sun.star.sheet.DataPilotFieldOrientation) aValue;
            if ( nDimension != ExampleSettings.nValueDimension &&
                 nDimension != ExampleSettings.nDataDimension &&
                 eNewOrient != com.sun.star.sheet.DataPilotFieldOrientation.DATA )
            {
                // remove from list for old orientation and add for new one
                Integer aDimInt = Integer.valueOf(nDimension);
                if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.COLUMN )
                    aSettings.aColDimensions.remove( aDimInt );
                else if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.ROW )
                    aSettings.aRowDimensions.remove( aDimInt );
                if ( eNewOrient == com.sun.star.sheet.DataPilotFieldOrientation.COLUMN )
                    aSettings.aColDimensions.add( aDimInt );
                else if ( eNewOrient == com.sun.star.sheet.DataPilotFieldOrientation.ROW )
                    aSettings.aRowDimensions.add( aDimInt );

                // change orientation
                eOrientation = eNewOrient;
            }
        }
        else if ( aPropertyName.equals( "Position" ) )
        {
            int nNewPos = ((Integer) aValue).intValue();
            Integer aDimInt = Integer.valueOf(nDimension);
            if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.COLUMN )
            {
                aSettings.aColDimensions.remove( aDimInt );
                aSettings.aColDimensions.add( nNewPos, aDimInt );
            }
            else if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.ROW )
            {
                aSettings.aRowDimensions.remove( aDimInt );
                aSettings.aRowDimensions.add( nNewPos, aDimInt );
            }
        }
        else if ( aPropertyName.equals( "Function" ) || aPropertyName.equals( "UsedHierarchy" ) ||
                  aPropertyName.equals( "Filter" ) )
        {
            // ignored
        }
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public Object getPropertyValue( String aPropertyName )
                    throws com.sun.star.beans.UnknownPropertyException
    {
        if ( aPropertyName.equals( "Original" ) )
            return null;
        else if ( aPropertyName.equals( "IsDataLayoutDimension" ) )
            return Boolean.valueOf( nDimension == ExampleSettings.nDataDimension );
        else if ( aPropertyName.equals( "Orientation" ) )
            return eOrientation;
        else if ( aPropertyName.equals( "Position" ) )
        {
            int nPosition;
            if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.COLUMN )
                nPosition = aSettings.aColDimensions.indexOf( Integer.valueOf(nDimension) );
            else if ( eOrientation == com.sun.star.sheet.DataPilotFieldOrientation.ROW )
                nPosition = aSettings.aRowDimensions.indexOf( Integer.valueOf(nDimension) );
            else
                nPosition = nDimension;
            return Integer.valueOf( nPosition );
        }
        else if ( aPropertyName.equals( "Function" ) )
            return com.sun.star.sheet.GeneralFunction.SUM;
        else if ( aPropertyName.equals( "UsedHierarchy" ) )
            return Integer.valueOf(0);
        else if ( aPropertyName.equals( "Filter" ) )
            return new com.sun.star.sheet.TableFilterField[0];
        else
            throw new com.sun.star.beans.UnknownPropertyException();
    }

    public void addPropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener xListener)
    {
    }
    public void removePropertyChangeListener(
        String aPropertyName, com.sun.star.beans.XPropertyChangeListener aListener)
    {
    }
    public void addVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
    public void removeVetoableChangeListener(
        String PropertyName, com.sun.star.beans.XVetoableChangeListener aListener)
    {
    }
}

//  implementation of com.sun.star.sheet.DataPilotSourceDimensions

class ExampleDimensions implements com.sun.star.container.XNameAccess
{
    private final ExampleSettings aSettings;
    private ExampleDimension[] aDimensions;

    public ExampleDimensions( ExampleSettings aSet )
    {
        aSettings = aSet;
    }

    //  XNameAccess

    public com.sun.star.uno.Type getElementType()
    {
        return new com.sun.star.uno.Type( com.sun.star.container.XNamed.class );
    }

    public boolean hasElements()
    {
        return true;    // always has elements
    }

    public Object getByName( String aName )
        throws com.sun.star.container.NoSuchElementException
    {
        for (int i=0; i<ExampleSettings.nDimensionCount; i++)
            if ( aName.equals( ExampleSettings.aDimensionNames[i] ) )
            {
                if ( aDimensions == null )
                    aDimensions = new ExampleDimension[ ExampleSettings.nDimensionCount ];
                if ( aDimensions[i] == null )
                    aDimensions[i] = new ExampleDimension( aSettings, i );
                return aDimensions[i];
            }
        throw new com.sun.star.container.NoSuchElementException();
    }

    public String[] getElementNames()
    {
        String [] aNames = new String[ ExampleSettings.nDimensionCount ];
        for (int i=0; i<ExampleSettings.nDimensionCount; i++)
            aNames[ i ] = ExampleSettings.aDimensionNames[i];
        return aNames;
    }

    public boolean hasByName( String aName )
    {
        for (int i=0; i<ExampleSettings.nDimensionCount; i++)
            if ( aName.equals( ExampleSettings.aDimensionNames[i] ) )
                return true;
        return false;
    }
}

//  outer class for service implementation

public class ExampleDataPilotSource
{
    //  implementation of com.sun.star.sheet.DataPilotSource

    public static class _ExampleDataPilotSource implements
                        com.sun.star.sheet.XDimensionsSupplier,
                        com.sun.star.sheet.XDataPilotResults,
                        com.sun.star.util.XRefreshable,
                        com.sun.star.beans.XPropertySet,
                        com.sun.star.lang.XInitialization,
                        com.sun.star.lang.XServiceInfo
    {
        private static final String aServiceName = "com.sun.star.sheet.DataPilotSource";
        private static final String aImplName =  _ExampleDataPilotSource.class.getName();

        private final ExampleSettings aSettings = new ExampleSettings();
        private ExampleDimensions aDimensions;

        public _ExampleDataPilotSource( com.sun.star.lang.XMultiServiceFactory xFactory )
        {
        }

        //  XInitialization

        public void initialize( Object[] aArguments )
        {
            //  If the first argument (Source) is a number between 2 and 10,
            //  use it as member count, otherwise keep the default value.
            try
            {
        if ( aArguments.length >= 1 )
        {
            String aSource = com.sun.star.uno.AnyConverter.toString(aArguments[0]);
            if ( aSource != null && aSource.length() > 0)
            {
            int nValue = Integer.parseInt( aSource );
            if ( nValue >= 2 && nValue <= 10 )
                aSettings.nMemberCount = nValue;
            }
        }
        }
        catch ( NumberFormatException e )
        {
        System.out.println( "Error: caught exception in " +
                    "ExampleDataPilotSource.initialize!\nException Message = "
                    + e.getMessage());
        e.printStackTrace();
        }
        catch ( com.sun.star.lang.IllegalArgumentException e )
        {
        System.out.println( "Error: caught exception in " +
                    "ExampleDataPilotSource.initialize!\nException Message = "
                    + e.getMessage());
        e.printStackTrace();
        }
        }

        //  XDataPilotResults

        public com.sun.star.sheet.DataResult[][] getResults()
        {
            int[] nDigits = new int[ExampleSettings.nDimensionCount];
            int nValue = 1;
            for (int i=0; i<ExampleSettings.nDimensionCount; i++)
            {
                nDigits[i] = nValue;
                nValue *= 10;
            }

            int nMemberCount = aSettings.nMemberCount;
            int nRowDimCount = aSettings.aRowDimensions.size();
            int nColDimCount = aSettings.aColDimensions.size();

            int nRows = 1;
            for (int i=0; i<nRowDimCount; i++)
                nRows *= nMemberCount;
            int nColumns = 1;
            for (int i=0; i<nColDimCount; i++)
                nColumns *= nMemberCount;

            com.sun.star.sheet.DataResult[][] aResults = new com.sun.star.sheet.DataResult[nRows][];
            for (int nRow=0; nRow<nRows; nRow++)
            {
                int nRowVal = nRow;
                int nRowResult = 0;
                for (int nRowDim=0; nRowDim<nRowDimCount; nRowDim++)
                {
                    int nDim = aSettings.aRowDimensions.get(nRowDimCount-nRowDim-1).intValue();
                    nRowResult += ( nRowVal % nMemberCount ) * nDigits[nDim];
                    nRowVal /= nMemberCount;
                }

                aResults[nRow] = new com.sun.star.sheet.DataResult[nColumns];
                for (int nCol=0; nCol<nColumns; nCol++)
                {
                    int nColVal = nCol;
                    int nResult = nRowResult;
                    for (int nColDim=0; nColDim<nColDimCount; nColDim++)
                    {
                        int nDim = aSettings.aColDimensions.get(nColDimCount-nColDim-1).intValue();
                        nResult += ( nColVal % nMemberCount ) * nDigits[nDim];
                        nColVal /= nMemberCount;
                    }

                    aResults[nRow][nCol] = new com.sun.star.sheet.DataResult();
                    aResults[nRow][nCol].Flags = com.sun.star.sheet.DataResultFlags.HASDATA;
                    aResults[nRow][nCol].Value = nResult;
                }
            }
            return aResults;
        }

        public double[] getFilteredResults(DataPilotFieldFilter[] aFilters) {
            // FIXME
            return new double[0];
        }

        //  XDimensionsSupplier

        public com.sun.star.container.XNameAccess getDimensions()
        {
            if ( aDimensions == null )
                aDimensions = new ExampleDimensions( aSettings );
            return aDimensions;
        }

        //  XPropertySet

        public com.sun.star.beans.XPropertySetInfo getPropertySetInfo()
        {
            return new ExamplePropertySetInfo( new com.sun.star.beans.Property[] {
                new com.sun.star.beans.Property( "ColumnGrand", -1,
                       new com.sun.star.uno.Type( Boolean.class ), (short) 0),
                new com.sun.star.beans.Property( "RowGrand", -1,
                       new com.sun.star.uno.Type( Boolean.class ), (short) 0) });
        }

        public void setPropertyValue( String aPropertyName, Object aValue )
            throws com.sun.star.beans.UnknownPropertyException
        {
            if ( aPropertyName.equals( "ColumnGrand" ) ||
                 aPropertyName.equals( "RowGrand" ) )
            {
                // ignored
            }
            else
                throw new com.sun.star.beans.UnknownPropertyException();
        }

        public Object getPropertyValue( String aPropertyName )
            throws com.sun.star.beans.UnknownPropertyException
        {
            if ( aPropertyName.equals( "ColumnGrand" ) ||
                 aPropertyName.equals( "RowGrand" ) )
            {
                return Boolean.FALSE;        // always false
            }
            else
                throw new com.sun.star.beans.UnknownPropertyException();
        }

        public void addPropertyChangeListener(
            String aPropertyName,
            com.sun.star.beans.XPropertyChangeListener xListener )
        {
        }
        public void removePropertyChangeListener(
            String aPropertyName,
            com.sun.star.beans.XPropertyChangeListener aListener )
        {
        }
        public void addVetoableChangeListener(
            String PropertyName,
            com.sun.star.beans.XVetoableChangeListener aListener )
        {
        }
        public void removeVetoableChangeListener(
            String PropertyName,
            com.sun.star.beans.XVetoableChangeListener aListener )
        {
        }

        //  XRefreshable

        public void refresh()
        {
        }
        public void addRefreshListener( com.sun.star.util.XRefreshListener l )
        {
        }
        public void removeRefreshListener( com.sun.star.util.XRefreshListener l )
        {
        }

        //  XServiceInfo

        public String getImplementationName()
        {
            return aImplName;
        }

        public String[] getSupportedServiceNames()
        {
            String [] aSupportedServices = new String[ 1 ];
            aSupportedServices[ 0 ] = aServiceName;
            return aSupportedServices;
        }

        public boolean supportsService( String aService )
        {
            return aService.equals( aServiceName );
        }
    }

    public static com.sun.star.lang.XSingleServiceFactory __getServiceFactory(
        String implName,
        com.sun.star.lang.XMultiServiceFactory multiFactory,
        com.sun.star.registry.XRegistryKey regKey)
    {
        com.sun.star.lang.XSingleServiceFactory xSingleServiceFactory = null;

        if ( implName.equals(_ExampleDataPilotSource.aImplName) )
            xSingleServiceFactory =
                com.sun.star.comp.loader.FactoryHelper.getServiceFactory(
                    _ExampleDataPilotSource.class,
                    _ExampleDataPilotSource.aServiceName, multiFactory, regKey);

        return xSingleServiceFactory;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
