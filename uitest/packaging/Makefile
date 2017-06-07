all:
	@echo "Packaging the libreoffice-connection code"
	@mkdir -p libreoffice
	@cp ../libreoffice/connection.py libreoffice/.
	@touch libreoffice/__init__.py
	@python3 setup.py sdist

clean:
	rm -r dist/
	rm -r libreoffice_connection.egg-info/
	rm -r libreoffice/
	rm -r build/

publish:
	@echo "Uploading the release to pypi"
	twine upload dist/*
