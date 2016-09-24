package org.libreoffice.storage.external;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.support.annotation.Nullable;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import org.libreoffice.R;
import org.libreoffice.storage.IOUtils;

import java.io.File;
import java.util.ArrayList;
import java.util.Comparator;

/**
 * A simple directory browser.
 */
public class DirectoryBrowserFragment extends Fragment {
    private static final String LOGTAG = DirectoryBrowserFragment.class.getSimpleName();
    private static final String INITIAL_PATH_URI_KEY = "initial_path";
    private File currentDirectory;
    private FileArrayAdapter directoryAdapter;

    public static DirectoryBrowserFragment newInstance(String initialPathURI) {
        Bundle args = new Bundle();
        args.putString(INITIAL_PATH_URI_KEY, initialPathURI);
        DirectoryBrowserFragment fragment = new DirectoryBrowserFragment();
        fragment.setArguments(args);
        Log.d(LOGTAG, "Saved path: " + initialPathURI);

        return fragment;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String initialPathURI = getArguments().getString(INITIAL_PATH_URI_KEY);
        setupCurrentDirectory(initialPathURI);
    }

    @Nullable
    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        View v = inflater.inflate(R.layout.fragment_directory_browser, container, false);

        final EditText directoryHeader = (EditText)v.findViewById(R.id.directory_header);
        Button directorySearchButton = (Button)v.findViewById(R.id.directory_search_button);
        Button positiveButton = (Button)v.findViewById(R.id.confirm_button);
        Button negativeButton = (Button)v.findViewById(R.id.cancel_button);
        ImageView upImage = (ImageView)v.findViewById(R.id.up_image);
        ListView directoryListView = (ListView) v.findViewById(R.id.directory_list);

        directoryHeader.setText(currentDirectory.getPath());
        directorySearchButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                String currentPath = currentDirectory.getAbsolutePath();
                String enteredPath = directoryHeader.getText().toString();
                File testDirectory = new File(enteredPath);
                if(enteredPath.equals(currentPath)) ;
                else if (isInvalidFileDirectory(testDirectory)) {
                    Toast.makeText(getActivity(), R.string.bad_directory, Toast.LENGTH_SHORT)
                            .show();
                }
                else {
                   changeDirectory(testDirectory);
                }
            }
        });

        positiveButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent data = new Intent();
                data.setData(Uri.fromFile(currentDirectory));
                getActivity().setResult(Activity.RESULT_OK, data);
                getActivity().finish();
            }
        });

        negativeButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                getActivity().setResult(Activity.RESULT_CANCELED, null);
                getActivity().finish();
            }
        });

        upImage.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                changeDirectory(currentDirectory.getParentFile());
            }
        });

        directoryAdapter = new FileArrayAdapter(getActivity(), new ArrayList<File>());
        directoryAdapter.populateFileList(currentDirectory);
        directoryListView.setAdapter(directoryAdapter);
        directoryListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                changeDirectory(directoryAdapter.getItem(position));
            }
        });

        return v;
    }

    private void changeDirectory(File destination) {
        if(destination == null) {
            Toast.makeText(getActivity(), "Unable to go further.", Toast.LENGTH_SHORT)
                    .show();
        } else {
            Fragment fragment = DirectoryBrowserFragment.newInstance(destination.toURI().toString());
            getActivity().getFragmentManager().beginTransaction()
                    .replace(R.id.fragment_container, fragment)
                    .addToBackStack(null)
                    .commit();
        }
    }

    private void setupCurrentDirectory(String initialPathURI) {
        File initialDirectory = null;
        if(initialPathURI != null && !initialPathURI.isEmpty()) {
            initialDirectory = IOUtils.getFileFromURIString(initialPathURI);
        }

        if(isInvalidFileDirectory(initialDirectory)) {
            initialDirectory = Environment.getExternalStorageDirectory();
        }
        currentDirectory = initialDirectory;
    }

    private boolean isInvalidFileDirectory(File f) {
        return f == null || !f.exists() || !f.isDirectory() ||!f.canRead();
    }

    private class FileArrayAdapter extends ArrayAdapter<File> {
        private Comparator<File> caseInsensitiveNaturalOrderComparator;

        public FileArrayAdapter(Context context, ArrayList<File> files) {
            super(context, 0, files);
            caseInsensitiveNaturalOrderComparator = new AlphabeticalFileComparator();
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                convertView = getActivity().getLayoutInflater()
                        .inflate(android.R.layout.simple_list_item_1, null);
            }

            File f = this.getItem(position);
            TextView tv = (TextView) convertView.findViewById(android.R.id.text1);
            tv.setText(f.getName());

            return convertView;
        }

        public void sortAlphabetically() {
            this.sort(caseInsensitiveNaturalOrderComparator);
        }

        public void populateFileList(File directory) {
            for(File f : directory.listFiles()){
                if(f.isDirectory()){
                    directoryAdapter.add(f);
                }
            }
            directoryAdapter.sortAlphabetically();
        }
    }

    private class AlphabeticalFileComparator implements Comparator<File> {
        @Override
        public int compare(File lhs, File rhs) {
            String lhsName = lhs.getName();
            String rhsName = rhs.getName();

            return lhsName.compareToIgnoreCase(rhsName);
        }
    }
}
