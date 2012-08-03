// -*- Mode: Java; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
//
// This file is part of the LibreOffice project.
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// This is just a testbed for ideas and implementations. (Still, it might turn
// out to be somewhat useful as such while waiting for "real" apps.)

// Important points:

// Everything that might take a long time should be done asynchronously:
//  - loading the document (loadComponentFromURL())
//  - counting number of pages (getRendererCount())
//  - rendering a page (render())

// Unclear whether pages can be rendered in parallel. Probably best to
// serialize all the above in the same worker thread, for instance using
// AsyncTask.SERIAL_EXECUTOR.

// While a page is loading ideally should display some animated spinner (but
// for now just a static "please wait" text).

// Just three views are used for the pages: For the current page being viewed,
// the previous, and the next. This could be bumped higher, need to make the
// "3" into a parameter below.

package org.libreoffice.android;

import org.libreoffice.R;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.GestureDetector;
import android.view.Gravity;
import android.view.MotionEvent;
import android.view.ViewGroup;
import android.view.animation.Animation;
import android.view.animation.AnimationSet;
import android.view.animation.TranslateAnimation;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.ViewFlipper;
import android.widget.ViewSwitcher;
import android.view.MenuItem;
import android.content.Intent;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.BaseAdapter;
import android.view.View.OnClickListener;
// Obsolete? 
import android.widget.AdapterView.OnItemClickListener;
import android.widget.GridView;
import android.widget.AdapterView;
import android.widget.ProgressBar;

import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.graphics.BitmapFactory;

import com.polites.android.GestureImageView;

import com.sun.star.awt.Size;
import com.sun.star.awt.XBitmap;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XToolkit2;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XController;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.view.XRenderable;

import java.io.*;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

import org.libreoffice.android.Bootstrap;
import org.libreoffice.ui.LibreOfficeUIActivity;

public class DocumentLoader
    extends Activity
{
    private static final String TAG = "DocumentLoader";
    
    // Size of a small virtual (bitmap) device used to find out page count and
    // page sizes
    private static final int SMALLSIZE = 128;

    // We pre-render this many pages preceding and succeeding the currently
    // viewed one, i.e. the total number of rendered pages kept is
    // PAGECACHE_PLUSMINUS*2+1.
    private static int PAGECACHE_PLUSMINUS = 2;
    private static int PAGECACHE_SIZE = PAGECACHE_PLUSMINUS*2 + 1;

    long timingOverhead;
    XComponentContext context;
    XMultiComponentFactory mcf;
    XComponentLoader componentLoader;
    XToolkit2 toolkit;
    XDevice dummySmallDevice;
    Object doc;
    int pageCount;
    int currentPage;
    XRenderable renderable;

    GestureDetector gestureDetector;

    ViewGroup.LayoutParams matchParent;

    ViewFlipper flipper;
    
    DocumentViewer documentViewer;

    Bundle extras;
    
    LinearLayout ll ;
    LayoutInflater inflater ;

    class GestureListener
        extends GestureDetector.SimpleOnGestureListener
    {
        @Override
        public boolean onFling(MotionEvent event1,
                               MotionEvent event2,
                               float velocityX,
                               float velocityY)
        {
            Log.i(TAG, "onFling: " + event1 + " " + event2);
            if (event1.getX() - event2.getX() > 120) {
                if (((PageViewer)flipper.getCurrentView()).currentPageNumber == pageCount-1)
                    return false;

                Animation inFromRight = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 1, Animation.RELATIVE_TO_SELF, 0,
                                                               Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                inFromRight.setDuration(500);
                flipper.setInAnimation(inFromRight);

                Animation outToLeft = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, -1,
                                                             Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                outToLeft.setDuration(500);
                flipper.setOutAnimation(outToLeft);

                documentViewer.nextPage();
                //((PageViewer)flipper.getChildAt((flipper.getDisplayedChild() + PAGECACHE_PLUSMINUS) % PAGECACHE_SIZE)).display(((PageViewer)flipper.getCurrentView()).currentPageNumber + PAGECACHE_PLUSMINUS);
                return true;
            } else if (event2.getX() - event1.getX() > 120) {
                if (((PageViewer)flipper.getCurrentView()).currentPageNumber == 0)
                    return false;

                Animation inFromLeft = new TranslateAnimation(Animation.RELATIVE_TO_SELF, -1, Animation.RELATIVE_TO_SELF, 0,
                                                              Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                inFromLeft.setDuration(500);
                flipper.setInAnimation(inFromLeft);

                Animation outToRight = new TranslateAnimation(Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 1,
                                                              Animation.RELATIVE_TO_SELF, 0, Animation.RELATIVE_TO_SELF, 0);
                outToRight.setDuration(500);
                flipper.setOutAnimation(outToRight);

                documentViewer.prevPage();

                //((PageViewer)flipper.getChildAt((flipper.getDisplayedChild() + PAGECACHE_SIZE - PAGECACHE_PLUSMINUS) % PAGECACHE_SIZE)).display(((PageViewer)flipper.getCurrentView()).currentPageNumber - PAGECACHE_PLUSMINUS);

                return true;
            }
            return false;
        }
        
        @Override
        public boolean onSingleTapUp(MotionEvent event){
        	if( getActionBar().isShowing() ){
        		getActionBar().hide();
        	}else{
        		getActionBar().show();
        	}	
        	return true;
        }
        
        @Override
        public boolean onDoubleTap(MotionEvent event){
        LinearLayout ll = (LinearLayout)findViewById( R.id.navigator);
        	if( ll.isShown() ){
        		ll.setVisibility( View.GONE );
        	}else{
        		ll.setVisibility( View.VISIBLE );
        	}
        	return true;	
        }
        
        
    }

    class MyXController
        implements XController
    {

        XFrame frame;
        XModel model;

        public void attachFrame(XFrame frame)
        {
            Log.i(TAG, "attachFrame");
            this.frame = frame;
        }

        public boolean attachModel(XModel model)
        {
            Log.i(TAG, "attachModel");
            this.model = model;
            return true;
        }

        public boolean suspend(boolean doSuspend)
        {
            Log.i(TAG, "suspend");
            return false;
        }

        public Object getViewData()
        {
            Log.i(TAG, "getViewData");
            return null;
        }

        public void restoreViewData(Object data)
        {
            Log.i(TAG, "restoreViewData");
        }

        public XModel getModel()
        {
            Log.i(TAG, "getModel");
            return model;
        }

        public XFrame getFrame()
        {
            Log.i(TAG, "getFrame");
            return frame;
        }

        public void dispose()
        {
            Log.i(TAG, "dispose");
        }

        public void addEventListener(XEventListener listener)
        {
            Log.i(TAG, "addEventListener");
        }

        public void removeEventListener(XEventListener listener)
        {
            Log.i(TAG, "removeEventListener");
        }
    }

    ByteBuffer renderPage(int number)
    {
        try {
            // Use dummySmallDevice with no scale of offset just to find out
            // the paper size of this page.
            Log.i( TAG , "Render( " + Integer.toString( number ) + " )");

            if (renderable == null) {
                Log.i( TAG , "no renderable");
                return null;
            }

            PropertyValue renderProps[] = new PropertyValue[3];
            renderProps[0] = new PropertyValue();
            renderProps[0].Name = "IsPrinter";
            renderProps[0].Value = new Boolean(true);
            renderProps[1] = new PropertyValue();
            renderProps[1].Name = "RenderDevice";
            renderProps[1].Value = dummySmallDevice;
            renderProps[2] = new PropertyValue();
            renderProps[2].Name = "View";
            renderProps[2].Value = new MyXController();

            // getRenderer returns a set of properties that include the PageSize
            long t0 = System.currentTimeMillis();
            PropertyValue rendererProps[] = renderable.getRenderer(number, doc, renderProps);
            long t1 = System.currentTimeMillis();
            Log.i(TAG, "getRenderer took " + ((t1-t0)-timingOverhead) + " ms");

            int pageWidth = 0, pageHeight = 0;
            for (int i = 0; i < rendererProps.length; i++) {
                if (rendererProps[i].Name.equals("PageSize")) {
                    pageWidth = ((Size) rendererProps[i].Value).Width;
                    pageHeight = ((Size) rendererProps[i].Value).Height;
                    Log.i(TAG, "PageSize: " + pageWidth + "x" + pageHeight);
                }
            }

            // Create a new device with the correct scale and offset
            ByteBuffer bb = ByteBuffer.allocateDirect(flipper.getWidth()*flipper.getHeight()*4);
            long wrapped_bb = Bootstrap.new_byte_buffer_wrapper(bb);

            XDevice device;
            if (pageWidth == 0) {
                // Huh?
                device = toolkit.createScreenCompatibleDeviceUsingBuffer(flipper.getWidth(), flipper.getHeight(), 1, 1, 0, 0, wrapped_bb);
            } else {

                // Scale so that it fits our device which has a resolution of 96/in (see
                // SvpSalGraphics::GetResolution()). The page size returned from getRenderer() is in 1/mm * 100.

                int scaleNumerator, scaleDenominator;

                // If the view has a wider aspect ratio than the page, fit
                // height; otherwise, fit width
                if ((double) flipper.getWidth() / flipper.getHeight() > (double) pageWidth / pageHeight) {
                    scaleNumerator = flipper.getHeight();
                    scaleDenominator = pageHeight / 2540 * 96;
                } else {
                    scaleNumerator = flipper.getWidth();
                    scaleDenominator = pageWidth / 2540 * 96;
                }
                Log.i(TAG, "Scaling with " + scaleNumerator + "/" + scaleDenominator);

                device = toolkit.createScreenCompatibleDeviceUsingBuffer(flipper.getWidth(), flipper.getHeight(),
                                                                         scaleNumerator, scaleDenominator,
                                                                         0, 0,
                                                                         wrapped_bb);
            }

            // Update the property that points to the device
            renderProps[1].Value = device;

            t0 = System.currentTimeMillis();
            renderable.render(number, doc, renderProps);
            t1 = System.currentTimeMillis();
            Log.i(TAG, "Rendering page " + number + " took " + ((t1-t0)-timingOverhead) + " ms");

            Bootstrap.force_full_alpha_bb(bb, 0, flipper.getWidth() * flipper.getHeight() * 4);

            return bb;
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }

        return null;
    }

    ByteBuffer renderPage(int number, int width , int height)
    {
        try {
            // Use dummySmallDevice with no scale of offset just to find out
            // the paper size of this page.

            if (renderable == null) {
                Log.i( TAG , "no renderable to render page ( " + Integer.toString(number) + " )" );
                return null;
            }

            PropertyValue renderProps[] = new PropertyValue[3];
            renderProps[0] = new PropertyValue();
            renderProps[0].Name = "IsPrinter";
            renderProps[0].Value = new Boolean(true);
            renderProps[1] = new PropertyValue();
            renderProps[1].Name = "RenderDevice";
            renderProps[1].Value = dummySmallDevice;
            renderProps[2] = new PropertyValue();
            renderProps[2].Name = "View";
            renderProps[2].Value = new MyXController();

            // getRenderer returns a set of properties that include the PageSize
            long t0 = System.currentTimeMillis();
            PropertyValue rendererProps[] = renderable.getRenderer(number, doc, renderProps);
            long t1 = System.currentTimeMillis();
            Log.i(TAG, "w,h getRenderer took " + ((t1-t0)-timingOverhead) + " ms");

            int pageWidth = 0, pageHeight = 0;
            for (int i = 0; i < rendererProps.length; i++) {
                if (rendererProps[i].Name.equals("PageSize")) {
                    pageWidth = ((Size) rendererProps[i].Value).Width;
                    pageHeight = ((Size) rendererProps[i].Value).Height;
                    Log.i(TAG, " w,h PageSize: " + pageWidth + "x" + pageHeight);
                }
            }

            // Create a new device with the correct scale and offset
            ByteBuffer bb = ByteBuffer.allocateDirect(width*height*4);
            long wrapped_bb = Bootstrap.new_byte_buffer_wrapper(bb);

            XDevice device;
            if (pageWidth == 0) {
                // Huh?
                device = toolkit.createScreenCompatibleDeviceUsingBuffer(width, height, 1, 1, 0, 0, wrapped_bb);
            } else {

                // Scale so that it fits our device which has a resolution of 96/in (see
                // SvpSalGraphics::GetResolution()). The page size returned from getRenderer() is in 1/mm * 100.

                int scaleNumerator, scaleDenominator;

                // If the view has a wider aspect ratio than the page, fit
                // height; otherwise, fit width
                if ((double) width / height > (double) pageWidth / pageHeight) {
                    scaleNumerator = height;
                    scaleDenominator = pageHeight / 2540 * 96;
                } else {
                    scaleNumerator = width;
                    scaleDenominator = pageWidth / 2540 * 96;
                }
                Log.i(TAG, "w,h Scaling with " + scaleNumerator + "/" + scaleDenominator);

                device = toolkit.createScreenCompatibleDeviceUsingBuffer(width, height,
                                                                         scaleNumerator, scaleDenominator,
                                                                         0, 0,
                                                                         wrapped_bb);
            }

            // Update the property that points to the device
            renderProps[1].Value = device;

            t0 = System.currentTimeMillis();
            renderable.render(number, doc, renderProps);
            t1 = System.currentTimeMillis();
            Log.i(TAG, "w,h Rendering page " + number + " took " + ((t1-t0)-timingOverhead) + " ms");

            Bootstrap.force_full_alpha_bb(bb, 0, width * height * 4);

            return bb;
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }

        return null;
    }

    enum PageState { NONEXISTENT, LOADING, READY };

    class PageViewer
        extends ViewSwitcher
    {
        public int currentPageNumber = -1;
        TextView waitView;
        PageState state = PageState.NONEXISTENT;
        Bitmap bm;
        static final String TAG = "PAGE_VIEWER";
        int width ;
        int height;

        class PageLoadTask
            extends AsyncTask<Integer, Void, Integer>
        {
            protected Integer doInBackground(Integer... params)
            {
                Log.i( PageViewer.TAG, "doInBackground: page " + params[0].toString() );
                int number = params[0];

                if (number >= pageCount)
                    return -1;

                state = PageState.LOADING;
                currentPageNumber = number;
                ByteBuffer bb = renderPage(currentPageNumber , params[1] , params[2]);//
                bm = Bitmap.createBitmap( width, height, Bitmap.Config.ARGB_8888);
                bm.copyPixelsFromBuffer(bb);

                return currentPageNumber;
            }

            protected void onPostExecute(Integer result)
            {
                Log.i(PageViewer.TAG, "onPostExecute: " + result);
                if (result == -1)
                    return;

                ImageView imageView = new ImageView(DocumentLoader.this);
                imageView.setImageBitmap(bm);

                imageView.setScaleY(-1);

                if (getChildCount() == 2)
                    removeViewAt(1);
                addView(imageView, 1, matchParent);
                showNext();
                state = PageState.READY;
            }
        }

        void display(int number)
        {
            Log.i(this.TAG, "PageViewer display(" + number + ")");
            Log.i(this.TAG, "IF");
            if (number >= 0){
                waitView = new TextView(DocumentLoader.this);
                waitView.setText("Page " + (number+1) + ", wait...");
                addView(waitView, 0, matchParent);
            }
            Log.i(this.TAG, "ENDIF");
            currentPageNumber = number;
            state = PageState.NONEXISTENT;
            Log.i(this.TAG, "IF");
            if (getDisplayedChild() == 1) {
                showPrevious();
                removeViewAt(1);
            }
            Log.i(TAG, "ENDIF");
            if (number >= 0) {
                Log.i( this.TAG , "Loading " + Integer.toString( number ) + " on Async ");
                new PageLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, number , width , height);
            }else{
                Log.i( this.TAG , "Bad computer, Bold!");
            }
        }

        int getPage(){
        	return this.currentPageNumber;
        }
        
        PageViewer(int number , int width , int height)
        {
            super(DocumentLoader.this);
            if( number < 0)
            	return;
            this.width = width ;
            this.height = height;
            waitView = new TextView(DocumentLoader.this);
            waitView.setTextSize(24);
            waitView.setGravity(Gravity.CENTER);
            waitView.setBackgroundColor(Color.WHITE);
            waitView.setTextColor(Color.BLACK);

            display(number);
        }
    }

    class ThumbnailView
        extends ViewSwitcher
    {
        int currentPageNumber = -1;
        TextView waitView;
        View thumbnailView;
        //PageState state = PageState.NONEXISTENT;
        Bitmap bm;
        final float scale;
        final int widthInPx;
        final int heightInPx;
        final int defaultWidthPx = 120;
        final int defaultHeightPx = 120;
        final int thumbnailPaddingDp = 10;
        		
        class ThumbLoadTask
            extends AsyncTask<Integer, Void, Integer>
        {
            protected Integer doInBackground(Integer... params)
            {
                int number = params[0];

                if (number >= pageCount)
                    return -1;

                //state = PageState.LOADING;
                ByteBuffer bb = renderPage( number , widthInPx , heightInPx);
                bm = Bitmap.createBitmap( widthInPx , heightInPx , Bitmap.Config.ARGB_8888);
                bm.copyPixelsFromBuffer(bb);

                return number;
            }

            protected void onPostExecute(Integer result)
            {
                Log.i(TAG, "onPostExecute: " + result);
                if (result == -1)
                    return;

                //ImageView imageView = new ImageView(DocumentLoader.this);
                
				ImageView thumbImage = new ImageView(DocumentLoader.this);//(ImageView)findViewById( R.id.thumbnail );
                thumbImage.setImageBitmap(bm);
                int paddingPx = (int) (thumbnailPaddingDp* scale + 0.5f);
                thumbImage.setPadding( paddingPx , 0 , paddingPx , 0 );
                thumbImage.setScaleY(-1);

				Log.i( TAG, Integer.toString( thumbImage.getWidth() ) );
                if (getChildCount() == 1)
                    removeViewAt(0);
                addView(thumbImage, matchParent);
                showNext();
                //state = PageState.READY;
            }
        }

        void display(int number)
        {
            Log.i(TAG, "Thumbnail display(" + number + ")");
            if (number >= 0)
                waitView.setText("Page " + (number+1) + ", wait...");
            //state = PageState.NONEXISTENT;
            if (number >= 0) {
                new ThumbLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, number);
            }
        }

        Bitmap getBitmap(){
            return bm;
        }

        ThumbnailView(int number , int widthInDp , int heightInDp)
        {
            super(DocumentLoader.this);
            scale = getContext().getResources().getDisplayMetrics().density;
            widthInPx = (int) (widthInDp * scale + 0.5f);
            heightInPx = (int) (heightInDp* scale + 0.5f);
			waitView = new TextView( DocumentLoader.this );
            thumbnailView = inflater.inflate( R.layout.navigation_grid_item , null);

            display(number);
            
            
        }
        
        ThumbnailView(int number)
        {
            super(DocumentLoader.this);
            scale = getContext().getResources().getDisplayMetrics().density;
            widthInPx = defaultWidthPx;
            heightInPx = defaultHeightPx;
			waitView = new TextView( DocumentLoader.this );
            thumbnailView = inflater.inflate( R.layout.navigation_grid_item , null);

            display(number);
            
            
        }
    }

    class DocumentLoadTask
        extends AsyncTask<String, Integer, Integer>
    {
    	ProgressBar progressView; 

        protected void onPreExecute (){//TODO have another go at putting in a progress bar (shouldn't waste time on it now)
            matchParent = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    		TextView waitView = new TextView(DocumentLoader.this);
            waitView.setTextSize(24);
            waitView.setGravity(Gravity.CENTER);
            waitView.setBackgroundColor(Color.WHITE);
            waitView.setTextColor(Color.BLACK);
    		waitView.setText("Page " + (1) + ", wait...");

            progressView = new ProgressBar( DocumentLoader.this, null, android.R.attr.progressBarStyleHorizontal);
            progressView.setProgress( 10 );

            //flipper = new ViewFlipper(DocumentLoader.this);
            //flipper = (ViewFlipper)findViewById( R.id.page_flipper );
            //flipper.addView( waitView , 0 , matchParent);
            //flipper.showNext();
            currentPage = 0;
    	}
        
    	protected Integer doInBackground(String... params)
        {
            try {
                String url = params[0];
                Log.i(TAG, "Attempting to load " + url);

                PropertyValue loadProps[] = new PropertyValue[3];
                loadProps[0] = new PropertyValue();
                loadProps[0].Name = "Hidden";
                loadProps[0].Value = new Boolean(true);
                loadProps[1] = new PropertyValue();
                loadProps[1].Name = "ReadOnly";
                loadProps[1].Value = new Boolean(true);
                loadProps[2] = new PropertyValue();
                loadProps[2].Name = "Preview";
                loadProps[2].Value = new Boolean(true);

                long t0 = System.currentTimeMillis();
                doc = componentLoader.loadComponentFromURL(url, "_blank", 0, loadProps);
                publishProgress( new Integer( 33 ));
                long t1 = System.currentTimeMillis();
                Log.i(TAG, "Loading took " + ((t1-t0)-timingOverhead) + " ms => " +
                      (doc == null ? "null" : "have") + "document");

                Object toolkitService = mcf.createInstanceWithContext
                    ("com.sun.star.awt.Toolkit", context);
                toolkit = (XToolkit2) UnoRuntime.queryInterface(XToolkit2.class, toolkitService);

                renderable = (XRenderable) UnoRuntime.queryInterface(XRenderable.class, doc);

                // Set up dummySmallDevice and use it to find out the number
                // of pages ("renderers").
                ByteBuffer smallbb = ByteBuffer.allocateDirect(SMALLSIZE*SMALLSIZE*4);
                long wrapped_smallbb = Bootstrap.new_byte_buffer_wrapper(smallbb);
                dummySmallDevice = toolkit.createScreenCompatibleDeviceUsingBuffer(SMALLSIZE, SMALLSIZE, 1, 1, 0, 0, wrapped_smallbb);

                PropertyValue renderProps[] = new PropertyValue[3];
                renderProps[0] = new PropertyValue();
                renderProps[0].Name = "IsPrinter";
                renderProps[0].Value = new Boolean(true);
                renderProps[1] = new PropertyValue();
                renderProps[1].Name = "RenderDevice";
                renderProps[1].Value = dummySmallDevice;
                renderProps[2] = new PropertyValue();
                renderProps[2].Name = "View";
                renderProps[2].Value = new MyXController();

                if (renderable != null)
                {
                    t0 = System.currentTimeMillis();
                    pageCount = renderable.getRendererCount(doc, renderProps);
                    t1 = System.currentTimeMillis();
                    Log.i(TAG, "getRendererCount: " + pageCount + ", took " + ((t1-t0)-timingOverhead) + " ms");
                }
                else
                {
                    pageCount = 1; // hack
                    Log.i(TAG, "no / null renderable interface!");
                }
            }
            catch (Exception e) {
                e.printStackTrace(System.err);
                finish();
            }
            return new Integer( pageCount );
        }

        protected void onProgressUpdate(Integer progress){
        	progressView.setProgress( progress.intValue() );
        }

        protected void onPostExecute(Integer result){
            Log.i(TAG, "onPostExecute: " + result);
            if (result == -1)
                return;
            //flipper = new ViewFlipper(this);
            //flipper = (ViewFlipper)findViewById( R.id.page_flipper );
            //matchParent = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
            //flipper.removeViewAt( 0 );
            documentViewer = new DocumentViewer( (ViewFlipper)findViewById( R.id.page_flipper ) );
            documentViewer.open(0);

            //currentPage = 0;
            //openPageWithPrefetching( currentPage );

            /*
            //open method? set current page = 0?
            flipper.addView(new PageViewer(0), 0, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(i+1), i+1, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(-1), PAGECACHE_PLUSMINUS + i+1, matchParent);
              */  
            ll = (LinearLayout)findViewById( R.id.navigator);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(
				Context.LAYOUT_INFLATER_SERVICE);

			
			
			for( int i = 0; i < result.intValue() ; i++ ){
				ThumbnailView thumb = new ThumbnailView( i , (int)(120.0f / Math.sqrt(2) ) , 120 );
				final int pos = i;
				thumb.setOnClickListener(new OnClickListener() {
			
					@Override
					public void onClick(View v) {
						// TODO Auto-generated method stub
						Log.d("nav" , Integer.toString( pos ) );
                        //openPageWithPrefetching( pos );
                        documentViewer.open( pos );
					}
				});
				ll.addView ( thumb );
			}
        }
    }

    class DocumentViewer
    {
        private int currentPage;
        private ViewFlipper viewFlipper;
        //int pageCount;
        private int lastPage;
        private final int firstPage = 0;
        private final int CACHE_PLUSMINUS = 2;
        private final int CACHE_SIZE = 2*CACHE_PLUSMINUS + 1;
        private ViewGroup.LayoutParams matchParent = new ViewGroup.LayoutParams(
            ViewGroup.LayoutParams.MATCH_PARENT,
            ViewGroup.LayoutParams.MATCH_PARENT);

        public DocumentViewer(ViewFlipper viewFlipper ){
            this.currentPage = 0;
            this.viewFlipper = viewFlipper;
            this.lastPage = pageCount-1;
            //viewFlipper.removeAllViews();
            //Not doing this here doesn't sit right can have uninit viewFlipper
            viewFlipper.removeAllViews();
            //need to pre-fetch
            for( int i = 0 ; i < this.CACHE_PLUSMINUS + 1 ; i++ ){//Load 0 + cache+- next pages
                if( i > lastPage)
                    break;
                viewFlipper.addView( new PageViewer( i , viewFlipper.getWidth() , viewFlipper.getHeight()) , i );
            }
        }

        public DocumentViewer(ViewFlipper viewFlipper, int n ){
            this.currentPage = n;
            this.viewFlipper = viewFlipper;
            this.lastPage = pageCount-1;
            //viewFlipper.removeAllViews();
            //need to pre-fetch
            int pos = 0;
            for(int i = n - this.CACHE_PLUSMINUS ; i < n + this.CACHE_PLUSMINUS ; i++, pos++ ){
                if( i < firstPage || i > lastPage)
                    continue;//Perhaps should continue on < and break on > but this seems neater
                viewFlipper.addView( new PageViewer( i , viewFlipper.getWidth() , viewFlipper.getHeight()) , pos );
            }
        }

        public void nextPage(){
            if( this.currentPage == lastPage ){
                return;
            }
            this.currentPage++;
            if( !( this.currentPage + this.CACHE_PLUSMINUS > lastPage) ){//don't remove view's if there are no more to add
                viewFlipper.removeViewAt( 0 );
                viewFlipper.addView(
                new PageViewer( this.currentPage + this.CACHE_PLUSMINUS, viewFlipper.getWidth() , viewFlipper.getHeight())
                    , this.CACHE_SIZE -1 );
            }
            viewFlipper.showNext();
        }

        public void prevPage(){
            if( this.currentPage == firstPage ){
                return;
            }
            this.currentPage--;
            if( !( this.currentPage - this.CACHE_PLUSMINUS < 0) ){//don't remove view's if there are no more to add
                viewFlipper.removeViewAt( CACHE_SIZE - 1 );
                viewFlipper.addView(
                    new PageViewer( this.currentPage - this.CACHE_PLUSMINUS , viewFlipper.getWidth() , viewFlipper.getHeight())
                    , 0 );
            }
            viewFlipper.showPrevious();
        }

        public void open( int newPage ){
            int diff = newPage - this.currentPage;
            Log.i( TAG , "open( N )");
            Log.i( TAG , "current page : " + Integer.toString( this.currentPage) );
            Log.i( TAG , "opening : " + Integer.toString( newPage ) + " of " + Integer.toString( pageCount ) );
            //diff = 0 -> do nothing
            //abs(diff) >= cachesize -> fill as in constructor
            //if curr > n : new_low foreach in cache ? # < new_low remove : from >n_upper add to end
            //else : upbound foreach in cache ? # >= upbound remove ; add to start
            if( diff == 0 )
                return;
            if( Math.abs(diff) >= CACHE_SIZE ){
                int pos = 0;
                viewFlipper.removeAllViews();
                for(int i = newPage - this.CACHE_PLUSMINUS ; i < newPage + this.CACHE_PLUSMINUS ; i++, pos++ ){
                    if( i < firstPage || i > lastPage)
                        continue;//Perhaps should continue on < and break on > but this seems neater
                    viewFlipper.addView( new PageViewer( i , viewFlipper.getWidth() , viewFlipper.getHeight()) , pos );
                }
            }

            if( diff > 0 ){ // new > curr
                int lowerBound = newPage - this.CACHE_PLUSMINUS;// of the new range
                int pos = 0;
                for(int i = 0 ; i < this.CACHE_SIZE ; i++ ){
                    if( i >= viewFlipper.getChildCount() )
                        break;
                    PageViewer page = (PageViewer)viewFlipper.getChildAt( i );
                    if( page.getPage() < lowerBound )
                        viewFlipper.removeViewAt( i );
                }
                //for others add to end.
                int numberRecycled = viewFlipper.getChildCount();
                for(int i = lowerBound ; i < newPage + this.CACHE_PLUSMINUS ; i++, pos++ ){
                    if( pos < numberRecycled)
                        continue;
                    if( i < firstPage || i > lastPage)
                        continue;
                    viewFlipper.addView( new PageViewer( i  , viewFlipper.getWidth() , viewFlipper.getHeight()), pos , matchParent );
                }
            }

            if( diff < 0 ){ // new < curr
                //FIXME The first 5 thumbnails are forwards only.
                int upperBound = newPage + this.CACHE_PLUSMINUS;
                int pos = 0;
                for(int i = 0 ; i < this.CACHE_SIZE ; i++ ){
                    if( i >= viewFlipper.getChildCount() )
                        break;
                    PageViewer page = (PageViewer)viewFlipper.getChildAt( i );
                    if( page.getPage() > upperBound )
                        viewFlipper.removeViewAt( i );
                }
                int numberRecycled = viewFlipper.getChildCount();
                for(int i = newPage - this.CACHE_PLUSMINUS ; i < upperBound ; i++, pos++ ){
                    if( this.CACHE_SIZE - pos < numberRecycled)
                        continue;
                    if( i < firstPage || i > lastPage)
                        continue;
                    viewFlipper.addView( new PageViewer( i , viewFlipper.getWidth() , viewFlipper.getHeight()), 0 , matchParent );
                }
            }
            viewFlipper.setDisplayedChild( this.CACHE_PLUSMINUS );
//            viewFlipper.showNext();
        }

        public ViewFlipper getFlipper(){
            return viewFlipper;
        }
    }

    static void dumpUNOObject(String objectName, Object object)
    {
        Log.i(TAG, objectName + " is " + (object != null ? object.toString() : "null"));

        if (object == null)
            return;

        XTypeProvider typeProvider = (XTypeProvider)
            UnoRuntime.queryInterface(XTypeProvider.class, object);
        if (typeProvider == null)
            return;

        Type[] types = typeProvider.getTypes();
        if (types == null)
            return;

        for (Type t : types)
            Log.i(TAG, "  " + t.getTypeName());
    }

    static void dumpBytes(String name, byte[] bytes, int offset)
    {
        if (bytes == null) {
            Log.i(TAG, name + " is null");
            return;
        }
        Log.i(TAG, name + ":");

        if (offset != 0)
            Log.i(TAG, "  (offset " + offset + ")");

        for (int i = offset; i < Math.min(bytes.length, offset+160); i += 16) {
            String s = "";
            for (int j = i; j < Math.min(bytes.length, i+16); j++)
                s = s + String.format(" %02x", bytes[j]);

            Log.i(TAG, s);
        }
    }

    static void dumpBytes(String name, ByteBuffer bytes, int offset)
    {
        if (bytes == null) {
            Log.i(TAG, name + " is null");
            return;
        }
        Log.i(TAG, name + ":");

        if (offset != 0)
            Log.i(TAG, "  (offset " + offset + ")");

        for (int i = offset; i < Math.min(bytes.limit(), offset+160); i += 16) {
            String s = "";
            for (int j = i; j < Math.min(bytes.limit(), i+16); j++)
                s = s + String.format(" %02x", bytes.get(j));

            Log.i(TAG, s);
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

		extras = getIntent().getExtras();

        gestureDetector = new GestureDetector(this, new GestureListener());

        try {
            long t0 = System.currentTimeMillis();
            long t1 = System.currentTimeMillis();
            timingOverhead = t1 - t0;

            Bootstrap.setup(this);

            Bootstrap.putenv("SAL_LOG=yes");

            // Load a lot of shlibs here explicitly in advance because that
            // makes debugging work better, sigh
            Bootstrap.dlopen("libvcllo.so");
            Bootstrap.dlopen("libmergedlo.so");
            Bootstrap.dlopen("libswdlo.so");
            Bootstrap.dlopen("libswlo.so");
            
            // Log.i(TAG, "Sleeping NOW");
            // Thread.sleep(20000);

            context = com.sun.star.comp.helper.Bootstrap.defaultBootstrap_InitialComponentContext();

            Log.i(TAG, "context is" + (context!=null ? " not" : "") + " null");

            mcf = context.getServiceManager();

            Log.i(TAG, "mcf is" + (mcf!=null ? " not" : "") + " null");

            String input = getIntent().getStringExtra("input");
            if (input == null)
                input = "/assets/test1.odt";

            // We need to fake up an argv, and the argv[0] even needs to
            // point to some file name that we can pretend is the "program".
            // setCommandArgs() will prefix argv[0] with the app's data
            // directory.

            String[] argv = { "lo-document-loader", input };

            Bootstrap.setCommandArgs(argv);

            Bootstrap.initVCL();

            Object desktop = mcf.createInstanceWithContext
                ("com.sun.star.frame.Desktop", context);

            Log.i(TAG, "desktop is" + (desktop!=null ? " not" : "") + " null");

            Bootstrap.initUCBHelper();

            componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, desktop);

            Log.i(TAG, "componentLoader is" + (componentLoader!=null ? " not" : "") + " null");

            setContentView( R.layout.document_viewer );
            // Load the wanted document
            new DocumentLoadTask().executeOnExecutor(AsyncTask.SERIAL_EXECUTOR, "file://" + input);

			/*
            //flipper = new ViewFlipper(this);
			flipper = (ViewFlipper)findViewById( R.id.page_flipper );
            matchParent = new ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

            // should have document load task publish pageCount before this happens: Shouldn't try to pre-fetch
            // non-existant pages && need to know how many navigation thumbnails to render.
            // Should put these on anothe serial executor?
            flipper.addView(new PageViewer(0), 0, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(i+1), i+1, matchParent);
            for (int i = 0; i < PAGECACHE_PLUSMINUS; i++)
                flipper.addView(new PageViewer(-1), PAGECACHE_PLUSMINUS + i+1, matchParent);
                
            ll = (LinearLayout)findViewById( R.id.navigator);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(
				Context.LAYOUT_INFLATER_SERVICE);

			
			
			for( int i = 0; i < 2 ; i++ ){
				ThumbnailView thumb = new ThumbnailView( i , (int)(120.0f / Math.sqrt(2) ) , 120 );
				final int pos = i;
				thumb.setOnClickListener(new OnClickListener() {
			
					@Override
					public void onClick(View v) {
						// TODO Auto-generated method stub
						Log.d("nav" , Integer.toString( pos ) );
						
					}
				});
				ll.addView ( thumb );
			}*/
            
	    	
        }
        catch (Exception e) {
            e.printStackTrace(System.err);
            finish();
        }
    }

    protected void onDestroy()
    {
        super.onDestroy();
        //Save the thumbnail of the first page as the grid image.
        // Could easily make a new (larger) thumb but recycling
        // should be faster & more efficient, better for the environment ;-)
        //ll = (LinearLayout)findViewById( R.id.navigator);
        Bitmap bmp = ( (ThumbnailView)ll.getChildAt( 0 ) ).getBitmap();
        File file = new File(extras.getString("input"));
        Log.i(TAG ,"onDestroy " + extras.getString("input"));
        File dir = file.getParentFile();
        String[] nameComponents = file.getName().split("[.]");
        for(String str : nameComponents){
            Log.i(TAG,"onDestroy " + str);
        }
        File thumbnailFile = new File( dir , "." + file.getName().split("[.]")[0] + ".png");
        try {
            ByteArrayOutputStream bytes = new ByteArrayOutputStream();
            bmp.compress(Bitmap.CompressFormat.JPEG, 40, bytes);
            thumbnailFile.createNewFile();
            FileOutputStream fo = new FileOutputStream( thumbnailFile );
            fo.write(bytes.toByteArray());
        } catch (IOException e) {
            // TODO: handle exception
        }
    }

    @Override
    public boolean onTouchEvent(MotionEvent event)
    {
        return gestureDetector.onTouchEvent(event);
    }
    
    @Override
	public boolean onOptionsItemSelected(MenuItem item) {
	    switch (item.getItemId()) {
	        case android.R.id.home:
	            // app icon in action bar clicked; go home
	            Intent intent = new Intent(this, LibreOfficeUIActivity.class);
	            intent.putExtras( extras );
	            //intent.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
	            startActivity(intent);
	            return true;
	        default:
	            return super.onOptionsItemSelected(item);
	    }
	}
    
}

// vim:set shiftwidth=4 softtabstop=4 expandtab:
