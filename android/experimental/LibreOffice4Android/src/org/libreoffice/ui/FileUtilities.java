package org.libreoffice.ui;

import java.io.File;
import java.io.FileFilter;
import java.io.FilenameFilter;
import java.util.Arrays;
import java.util.Comparator;

public class FileUtilities {
	
	static final int ALL = -1;
	static final int DOC = 0;
	static final int CALC = 1;
	static final int IMPRESS = 2;
	
	static final int SORT_AZ = 0;
	static final int SORT_ZA = 1;
	/** Oldest Files First*/
	static final int SORT_OLDEST = 2;
	/** Newest Files First*/
	static final int SORT_NEWEST = 3;
	/** Largest Files First */
	static final int SORT_LARGEST = 4;
	/** Smallest Files First */
	static final int SORT_SMALLEST = 5;
	
	private static String[] fileExtensions = {".odt",".ods",".odp"};
	
	static boolean isDoc(String filename){
		if( filename.endsWith( fileExtensions[ DOC ] ) ){
			return true;
		}
		return false;
	}
	
	static boolean isCalc(String filename){
		if( filename.endsWith( fileExtensions[ CALC ] ) ){
			return true;
		}
		return false;
	}
	
	static boolean isImpress(String filename){
		if( filename.endsWith( fileExtensions[ IMPRESS ] ) ){
			return true;
		}
		return false;
	}
	
	static FileFilter getFileFilter(int mode ){
		if( mode != ALL){
			final String ext = fileExtensions[ mode ];
			return new FileFilter() {
				
				public boolean accept(File pathname) {
					if( pathname.getName().endsWith( ext ) ){
						return true;
					}
					if( pathname.isDirectory() ){
						return true;
					}
					return false;
				}
			};
		}else{//return all
			return new FileFilter() {
				
				public boolean accept(File pathname) {
					// TODO Auto-generated method stub
					return true;
				}
			};
		}
	}

	static FilenameFilter getFilenameFilter(int mode){
		if( mode != ALL){
			final String ext = fileExtensions[ mode ];
			return new FilenameFilter() {
				
				public boolean accept(File dir, String filename) {
					if( filename.endsWith( ext ) ){
						return true;
					}
					if( new File( dir , filename ).isDirectory() ){
						return true;
					}
					return false;
				}
			};
		}else{
			return new FilenameFilter() {
				
				public boolean accept(File dir, String filename) {
					return true;
				}
			}; 
		}
	}

	static void sortFiles(File[] files , int sortMode){
		//Should  really change all this to a switch statement... 
		if( sortMode == SORT_AZ ){
			Arrays.sort( files , new Comparator<File>() {
	
				public int compare(File lhs, File rhs) {
					return lhs.getName().compareTo( rhs.getName() );
				}
			});
			return;
		}
		if( sortMode == SORT_ZA ){
			Arrays.sort( files , new Comparator<File>() {
				
				public int compare(File lhs, File rhs) {
					return rhs.getName().compareTo( lhs.getName() );
				}
			});
			return;
		}
		if( sortMode == SORT_OLDEST ){
			Arrays.sort( files , new Comparator<File>() {
				
				public int compare(File lhs, File rhs) {
					return Long.valueOf( lhs.lastModified() ).compareTo( rhs.lastModified() );
				}
			});
			return;
		}
		if( sortMode == SORT_NEWEST ){
			Arrays.sort( files , new Comparator<File>() {
				
				public int compare(File lhs, File rhs) {
					return Long.valueOf( rhs.lastModified() ).compareTo( lhs.lastModified() );
				}
			});
			return;
		}
		if( sortMode == SORT_LARGEST ){
			Arrays.sort( files , new Comparator<File>() {
				
				public int compare(File lhs, File rhs) {
					return Long.valueOf( rhs.length() ).compareTo( lhs.length() );
				}
			});
			return;
		}
		if( sortMode == SORT_SMALLEST ){
			Arrays.sort( files , new Comparator<File>() {
				
				public int compare(File lhs, File rhs) {
					return Long.valueOf( lhs.length() ).compareTo( rhs.length() );
				}
			});
			return;
		}
		return;
	}
}
