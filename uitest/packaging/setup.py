from setuptools import setup, find_packages
from codecs import open
from os import path

here = path.abspath(path.dirname(__file__))

# Get the long description from the README file
with open(path.join(here, 'README.rst'), encoding='utf-8') as f:
long_description = f.read()

setup(
        name="libreoffice-connection",
        version="0.0.1",
        description="Connection code for LibreOffice's pyUNO",
        long_description=long_description,
        url="http://www.libreoffice.org",
        author="The LibreOffice developers",
        author_email="libreoffice@lists.freedesktop.org",
        license="MPL2",
        classifiers=[
            "Development Status :: 3 - Alpha",
            "Intended Audience :: Developers",
            "License :: OSI Approved :: Mozilla Public License 2.0 (MPL 2.0)",
            "Programming Language :: Python :: 3.2",
            "Programming Language :: Python :: 3.3",
            "Programming Language :: Python :: 3.4",
            "Programming Language :: Python :: 3.5",
            "Programming Language :: Python :: 3.6",
            "Topic :: Office/Business :: Office Suites",
            "Topic :: Software Development :: Libraries",
            ],
        keywords="office automation",
        packages=find_packages(),
)
