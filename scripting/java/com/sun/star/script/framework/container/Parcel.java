package com.sun.star.script.framework.container;

import  com.sun.star.script.framework.log.*;
import  com.sun.star.script.framework.browse.*;

import com.sun.star.container.*;
import com.sun.star.uno.Type;
import com.sun.star.lang.*;
import com.sun.star.frame.*;
import java.io.*;
import java.util.*;
import java.net.*;

public abstract class Parcel implements XNameContainer
{
    final private String[] macroList = { "user", "share", "document" };
    protected ParcelDescriptor parcel;
    protected String name;
    protected ParcelContainer parent;
    public Parcel( ParcelContainer parent, ParcelDescriptor desc, String parcelName )
    {
        this.parent = parent;
        this.parcel = desc;
        this.name = parcelName;
    }
    public String getName()
    {
        return name;
    }
    public java.lang.Object getByName( String aName ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        LogUtils.DEBUG("** Parcel.getByName");
        ScriptEntry script = null;
        try
        {
            if ( parcel != null && hasElements() )
            {
                ScriptEntry[] scripts = parcel.getScriptEntries();
                if ( scripts.length != 0 )
                {
                    for ( int index = 0; index < scripts.length; index++ )
                    {
                        if ( scripts[ index ].getLanguageName().equals( aName ) )
                        {
                            script = scripts[ index ];
                            break;
                        }
                    }
                }
            }
        }
        catch ( Exception e )
        {
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }
        if ( script == null )
        {
            throw new com.sun.star.container.NoSuchElementException("No script named " + aName );
        }
        ScriptMetaData data = new ScriptMetaData( this, script, null );

        if ( !parcel.getLanguage().equals("Java") )
        {
            // TODO better seperate the ScriptMetaData Object
            // so notion of having source file is better represented
            // maybe needs an entry/attribute in parcel-desc.xml
            LogUtils.DEBUG("**** attempting to load source " );
            data.loadSource();
        }
        return data;
    }
    public String[] getElementNames()
    {
        String[] results = new String[0];
        if ( parcel != null )
        {
            ScriptEntry[] scripts = parcel.getScriptEntries();
            results = new String[ scripts.length ];
            for ( int index = 0; index < scripts.length; index++ )
            {
                results[ index ] = scripts[ index ].getLanguageName();
            }
        }
        return results;
    }
    public boolean hasByName( String aName )
    {

        boolean result = true;
        Object containee  = null;
        try
        {
            containee = getByName( aName );
            if ( containee != null )
            {
                result = true;
            }
        }
        catch( Exception e )
        {
            result = false;
        }
        return result;
    }

    public com.sun.star.uno.Type getElementType() {
        // TODO at the moment this returns void indicating
        // type is unknown ( from UNO point of view this is correct )
        // but, maybe we want to have a private UNO interface
        //
        return new Type();
    }

    public boolean hasElements()
    {
        if ( parcel != null && parcel.getScriptEntries().length > 0 )
        {
            return true;
        }
        return false;
    }



    public ParcelDescriptor getParcelDescriptor()
    {
       return parcel;
    }

    public void replaceByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
   {
       // TODO check type of aElement
       // if not ok, throw IllegalArgument
       if ( parcel != null )
       {
           try
           {
               ScriptEntry script = (ScriptEntry)getByName( aName );
               if ( script != null )
               {
                   // TODO needs to remove source file ( if there is one )
                   parcel.removeScriptEntry( script );
                   // TODO needs to create source file ( if there is one )
                   parcel.addScriptEntry( script );
                   //parcel.write();
               }
               else
               {
                   throw new com.sun.star.container.NoSuchElementException("No script named " + aName );
               }


           }
           // TO DO should catch specified exceptions
           catch ( Exception e )
           {
               throw new com.sun.star.lang.WrappedTargetException();
           }

       }
   }
    // create
    public void insertByName( String aName, java.lang.Object aElement ) throws com.sun.star.lang.IllegalArgumentException, ElementExistException, com.sun.star.lang.WrappedTargetException
    {
        // TODO check the type of aElement and throw#
        // if not appropriate
        try
        {
            if ( hasByName( aName ) )
            {
                throw new ElementExistException( aName );
            }
            ScriptMetaData script = (ScriptMetaData)aElement;

            if (  script.hasSource() )
            {
                if ( !script.writeSourceFile() )
                {
                    throw new com.sun.star.lang.WrappedTargetException( "Failed to create source file " + script.getLanguageName() );
                }
            }
            parcel.addScriptEntry( script );
        }
        catch ( Exception e )
        {
            throw new com.sun.star.lang.WrappedTargetException( e.toString() );
        }

    }
    // delete
    public void removeByName( String Name ) throws com.sun.star.container.NoSuchElementException, com.sun.star.lang.WrappedTargetException
    {
        try
        {
            ScriptMetaData script = null;
            if ( ( script = (ScriptMetaData)getByName( Name ) ) != null )
            {
                if ( script.hasSource() )
                {
                   if ( !script.removeSourceFile() )
                   {
                       LogUtils.DEBUG("Failed to remove script" );
                       throw new com.sun.star.lang.WrappedTargetException("Failed to remove script " + Name );
                   }
                   LogUtils.DEBUG("** have remove script source file " + Name );
                }
                LogUtils.DEBUG("about to remove entry from descriptor");
                parcel.removeScriptEntry( script );
                LogUtils.DEBUG("removed entry from descriptor");
            }
            else
            {
                throw new com.sun.star.container.NoSuchElementException( "No script named " + Name );
            }

        }
        catch ( Exception e )
        {
            throw new  com.sun.star.lang.WrappedTargetException( e.toString() );
        }

    }
    // rename parcel
    public void rename( String name ) throws com.sun.star.lang.WrappedTargetException
    {
        this.name = name;
    }

    public ParcelContainer getParent() { return parent; }
    protected abstract String getPathToParcel();

    protected String getParcelLocationMacro()
    {
        return parent.locationMacro;


    }

}

