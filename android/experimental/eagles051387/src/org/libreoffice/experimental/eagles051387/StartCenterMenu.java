package org.libreoffice.experimental.eagles051387;

import org.libreoffice.experimentalUI.eagles051387.R;
import android.app.ListActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

public class StartCenterMenu extends ListActivity
{
	static final String[] LOOPTIONS = new String[]
	{
		"Open", "Templates", "Text Document", "Spreadsheet", "Presentation", "Drawing", "database",
		"Formula"
	};

	@Override
	public void onCreate(Bundle savedInstanceState)
	{
	  super.onCreate(savedInstanceState);

	  setListAdapter(new ArrayAdapter<String>(this, R.layout.star_center_menu_list, LOOPTIONS));

	  ListView lv = getListView();
	  lv.setTextFilterEnabled(true);

	  lv.setOnItemClickListener(new OnItemClickListener()
	  {
	    public void onItemClick(AdapterView<?> parent, View view, int position, long id)
	    {
	      // When clicked, show a toast with the TextView text
	      Toast.makeText(getApplicationContext(), ((TextView) view).getText(),
	          Toast.LENGTH_SHORT).show();
	    }
	  });
	}
}