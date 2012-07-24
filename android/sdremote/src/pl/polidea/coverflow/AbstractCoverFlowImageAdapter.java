package pl.polidea.coverflow;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.Map;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ImageView;

/**
 * This class is an adapter that provides base, abstract class for images
 * adapter.
 *
 */
public abstract class AbstractCoverFlowImageAdapter extends BaseAdapter {

	/** The Constant TAG. */
	private static final String TAG = AbstractCoverFlowImageAdapter.class
	                .getSimpleName();

	/** The width. */
	private float width = 0;

	/** The height. */
	private float height = 0;

	/** The bitmap map. */
	private final Map<Integer, WeakReference<Bitmap>> bitmapMap = new HashMap<Integer, WeakReference<Bitmap>>();

	public AbstractCoverFlowImageAdapter() {
		super();
	}

	/**
	 * Set width for all pictures.
	 *
	 * @param width
	 *            picture height
	 */
	public synchronized void setWidth(final float width) {
		this.width = width;
	}

	/**
	 * Set height for all pictures.
	 *
	 * @param height
	 *            picture height
	 */
	public synchronized void setHeight(final float height) {
		this.height = height;
	}

	@Override
	public final Bitmap getItem(final int position) {
		final WeakReference<Bitmap> weakBitmapReference = bitmapMap
		                .get(position);
		if (weakBitmapReference != null) {
			final Bitmap bitmap = weakBitmapReference.get();
			if (bitmap == null) {
				Log.v(TAG, "Empty bitmap reference at position: " + position
				                + ":" + this);
			} else {
				Log.v(TAG, "Reusing bitmap item at position: " + position + ":"
				                + this);
				return bitmap;
			}
		}
		Log.v(TAG, "Creating item at position: " + position + ":" + this);
		final Bitmap bitmap = createBitmap(position);
		bitmapMap.put(position, new WeakReference<Bitmap>(bitmap));
		Log.v(TAG, "Created item at position: " + position + ":" + this);
		return bitmap;
	}

	/**
	 * Creates new bitmap for the position specified.
	 *
	 * @param position
	 *            position
	 * @return Bitmap created
	 */
	protected abstract Bitmap createBitmap(int position);

	/*
	 * (non-Javadoc)
	 *
	 * @see android.widget.Adapter#getItemId(int)
	 */
	@Override
	public final synchronized long getItemId(final int position) {
		return position;
	}

	/*
	 * (non-Javadoc)
	 *
	 * @see android.widget.Adapter#getView(int, android.view.View,
	 * android.view.ViewGroup)
	 */
	@Override
	public final synchronized ImageView getView(final int position,
	                final View convertView, final ViewGroup parent) {
		ImageView imageView;
		if (convertView == null) {
			final Context context = parent.getContext();
			Log.v(TAG, "Creating Image view at position: " + position + ":"
			                + this);
			imageView = new ImageView(context);
			imageView.setLayoutParams(new CoverFlow.LayoutParams((int) width,
			                (int) height));
		} else {
			Log.v(TAG, "Reusing view at position: " + position + ":" + this);
			imageView = (ImageView) convertView;
		}
		imageView.setImageBitmap(getItem(position));
		return imageView;
	}

}
