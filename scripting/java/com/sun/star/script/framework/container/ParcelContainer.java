package com.sun.star.script.framework.container;

import  com.sun.star.script.framework.log.*;
import com.sun.star.container.*;
import com.sun.star.uno.Type;
import com.sun.star.lang.*;
import com.sun.star.frame.*;
import java.io.*;
import java.util.*;

abstract public class ParcelContainer implements XNameAccess
{
    protected String language;
    protected String locationMacro;
    protected Collection parcels = new ArrayList(4);
    protected SFMacroHelper expander;

    public ParcelContainer( SFMacroHelper expander, String language )
    {
        this.expander = expander;
        this.language = language;
    }
    public Object getByName( String aName ) throws com.sun.star.container.NoSuchElementException, WrappedTargetException
    {
        Parcel parcel = null;
        try
        {
            if ( hasElements() )
            {
                Iterator iter = parcels.iterator();
                while ( iter.hasNext() )
                {
                    Parcel parcelToCheck = (Parcel)iter.next();

                    if ( parcelToCheck.getName().equals( aName ) )
                    {
                       parcel = parcelToCheck;
                       break;
                    }
                }
            }
        }
        catch ( Exception e)
        {
            throw new WrappedTargetException( e.toString() );
        }
        if ( parcel == null )
        {
            throw new com.sun.star.container.NoSuchElementException( aName );
        }
        return parcel;
    }
    public String[] getElementNames()
    {
        if ( hasElements() )
        {
            Parcel[] theParcels = (Parcel[])parcels.toArray( new Parcel[0] );
            String[] names = new String[ theParcels.length ];
            for ( int count = 0; count < names.length; count++ )
            {
                names[count] = theParcels[ count ].getName();
            }
            return names;
        }

        return new String[0];
    }
    public boolean hasByName( String aName )
    {
        boolean isFound = false;
        try
        {
            if ( getByName( aName ) != null )
            {
                isFound = true;
            }

        }
        catch ( Exception e )
        {
            //TODO - handle trace
        }
        return isFound;
    }
    public Type getElementType()
    {
        return new Type();
    }
    public boolean hasElements()
    {
        if ( parcels == null || parcels.isEmpty() )
        {
            return false;
        }
        return true;
    }
    public abstract XNameContainer createParcel(String name) throws ElementExistException, com.sun.star.lang.WrappedTargetException;

    public abstract boolean deleteParcel(String name) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException;
    public abstract void renameParcel(String oldName, String newName) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException;
    public abstract String getRootPath();
    public String getLanguage() { return language; }
}
